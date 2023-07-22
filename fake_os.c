#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include "fake_os.h"
#include "fake_cpu.h"

void FakeOS_init(FakeOS* os) {
  os->running=0;
  List_init(&os->ready);
  List_init(&os->waiting);
  List_init(&os->processes);
  List_init(&os->cpus);
  os->timer=0;
  os->schedule_fn=0;
}

void FakeOS_createProcess(FakeOS* os, FakeProcess* p) {
  // sanity check
  assert(p->arrival_time==os->timer && "time mismatch in creation");
  // we check that in the list of PCBs there is no
  // pcb having the same pid
  assert( (!os->running || os->running->pid!=p->pid) && "pid taken");

  ListItem* aux=os->ready.first;
  while(aux){
    FakePCB* pcb=(FakePCB*)aux;
    assert(pcb->pid!=p->pid && "pid taken");
    aux=aux->next;
  }

  aux=os->waiting.first;
  while(aux){
    FakePCB* pcb=(FakePCB*)aux;
    assert(pcb->pid!=p->pid && "pid taken");
    aux=aux->next;
  }

  // all fine, no such pcb exists
  FakePCB* new_pcb=(FakePCB*) malloc(sizeof(FakePCB));
  new_pcb->list.next=new_pcb->list.prev=0;
  new_pcb->pid=p->pid;
  new_pcb->events=p->events;

  assert(new_pcb->events.first && "process without events");

  // depending on the type of the first event
  // we put the process either in ready or in waiting
  ProcessEvent* e=(ProcessEvent*)new_pcb->events.first;
  switch(e->type){
  case CPU:
    List_pushBack(&os->ready, (ListItem*) new_pcb);
    break;
  case IO:
    List_pushBack(&os->waiting, (ListItem*) new_pcb);
    break;
  default:
    assert(0 && "illegal resource");
    ;
  }
}




void FakeOS_simStep(FakeOS* os){
  
  printf("************** TIME: %08d **************\n", os->timer);

  //scan process waiting to be started
  //and create all processes starting now
  ListItem* aux=os->processes.first;
  while (aux){
    FakeProcess* proc=(FakeProcess*)aux;
    FakeProcess* new_process=0;
    if (proc->arrival_time==os->timer){
      new_process=proc;
    }
    aux=aux->next;
    if (new_process) {
      printf("\tcreate pid:%d\n", new_process->pid);
      new_process=(FakeProcess*)List_detach(&os->processes, (ListItem*)new_process);
      FakeOS_createProcess(os, new_process);
      free(new_process);
    }
  }

  // scan waiting list, and put in ready all items whose event terminates
  aux=os->waiting.first;
  while(aux) {
    FakePCB* pcb=(FakePCB*)aux;
    aux=aux->next;
    ProcessEvent* e=(ProcessEvent*) pcb->events.first;
    printf("\twaiting pid: %d\n", pcb->pid);
    assert(e->type==IO);
    e->duration--;
    printf("\t\tremaining time:%d\n",e->duration);
    if (e->duration==0){
      printf("\t\tend burst\n");
      List_popFront(&pcb->events);
      free(e);
      List_detach(&os->waiting, (ListItem*)pcb);
      if (! pcb->events.first) {
        // kill process
        printf("\t\tend process\n");
        free(pcb);
      } else {
        //handle next event
        e=(ProcessEvent*) pcb->events.first;
        switch (e->type){
        case CPU:
          printf("\t\tmove to ready\n");
          List_pushBack(&os->ready, (ListItem*) pcb);
          break;
        case IO:
          printf("\t\tmove to waiting\n");
          List_pushBack(&os->waiting, (ListItem*) pcb);
          break;
        }
      }
    }
  }

  

  // decrement the duration of running
  // if event over, destroy event
  // and reschedule process
  // if last event, destroy running
  FakePCB* running=os->running;
  printf("\trunning pid: %d\n", running?running->pid:-1);
  if (running) {
    ProcessEvent* e=(ProcessEvent*) running->events.first;
    assert(e->type==CPU);
    e->duration--;
    printf("\t\tremaining time:%d\n",e->duration);
    if (e->duration==0){
      printf("\t\tend burst\n");
      List_popFront(&running->events);
      free(e);
      if (! running->events.first) {
        printf("\t\tend process\n");
        free(running); // kill process
      } else {
        e=(ProcessEvent*) running->events.first;
        switch (e->type){
        case CPU:
          printf("\t\tmove to ready\n");
          List_pushBack(&os->ready, (ListItem*) running);
          break;
        case IO:
          printf("\t\tmove to waiting\n");
          List_pushBack(&os->waiting, (ListItem*) running);
          break;
        }
      }
      os->running = 0;
    }
  }


  // call schedule, if defined
  if (os->schedule_fn && ! os->running){
    (*os->schedule_fn)(os, os->schedule_args); 
  }

  // if running not defined and ready queue not empty
  // put the first in ready to run
  if (! os->running && os->ready.first) {
    os->running=(FakePCB*) List_popFront(&os->ready);
  }

  ++os->timer;

}

void FakeOS_destroy(FakeOS* os) {

  // Here we free allocated memory used for cpu instances
  ListItem* aux = os->cpus.first;
  printf("Deallocating all cpus...");
  while(aux) {
    FakeCPU* cpu = (FakeCPU*) aux;
    free(cpu);
    aux = aux->next;
  }
  printf("DONE.\n");
}

int FakeOS_cpu_init(FakeOS* os, const char* filename) {
  FILE* cpu_file = fopen(filename, "r");
  if(!cpu_file) {
    return -1;
  }
  char *buffer = NULL;
  size_t line_length = 0;
  int num_cpu = 0;

  while(getline(&buffer, &line_length, cpu_file) > 0 ) {
    int cpu_id = -1;
    int schedule_algo = -0x01;
    int num_tokens = 0;

    // Dynamic allocation of FakeCPU instance
    FakeCPU* cpu = malloc(sizeof(FakeCPU));

    num_tokens = sscanf(buffer, "CPU %d %d", &cpu_id, &schedule_algo);
    if(num_tokens != 2) {
      break;
    }
    cpu->id = cpu_id;
    cpu->policy = (SchedulingAlgorithm) schedule_algo;
    cpu->status = IDLE;
    cpu->running = NULL;
    List_pushBack(&os->cpus, (ListItem*) cpu);
    num_cpu++;
  }

  if(buffer) {
    free(buffer);
  }
  fclose(cpu_file);
  return num_cpu;
}

void FakeOS_print_cpu(FakeOS* os) {
  ListItem* aux = os->cpus.first;
  while(aux) {
    FakeCPU* cpu = (FakeCPU*) aux;
    printf("id:%d, status:%d, policy:%d\n", cpu->id, cpu->status, cpu->policy);
    aux = aux->next;
  }
}
