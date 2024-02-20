#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include "fake_os.h"

#ifndef DEFAULT_CPUS
#define DEFAULT_CPUS 1
#endif

#define DEFAULT_PREDICTED_BURST 5

void FakeOS_init(FakeOS* os) {
  List_init(&os->ready);
  List_init(&os->waiting);
  List_init(&os->processes);
  List_init(&os->cpus);
  os->timer=0;
  os->schedule_fn=0;
  os->num_cpus=DEFAULT_CPUS;
}

void FakeOS_createProcess(FakeOS* os, FakeProcess* p) {
  // sanity check
  assert(p->arrival_time==os->timer && "time mismatch in creation");
  // we check that in the list of PCBs there is no
  // pcb having the same pid
  ListItem* aux=os->cpus.first;
  while (aux) {
    FakeCPU* cpu = (FakeCPU*) aux;
    assert(!cpu->running || cpu->running->pid!=p->pid && "pid taken");
    aux = aux->next; 
  }

  aux=os->ready.first;
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
  new_pcb->predicted_burst = DEFAULT_PREDICTED_BURST;
  new_pcb->running_time = 0;
  new_pcb->update_prediction = 0;

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
        printf("\t\tend process [%d]\n", pcb->pid);
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
  
  aux=os->cpus.first;
  while (aux) {
    FakeCPU* cpu = (FakeCPU*) aux;
    FakePCB* running = (FakePCB*) cpu->running;
    printf("\tCPU[%d]->running pid: %d\n",cpu->id, running?running->pid:-1);

    // le operazioni di decremento/reschedule/distruzione 
    // sono fatte per ogni processo in esecuzione su ogni cpu
    if (running) { 
      ProcessEvent* e=(ProcessEvent*) running->events.first;
      assert(e->type==CPU);
      running->running_time++;
      e->duration--;
      printf("\t\tremaining time:%d\n",e->duration);
      if (e->duration==0){
        printf("\t\tend burst\n");
        List_popFront(&running->events);
        free(e);
        FakeCPU_to_idle(cpu);
        if (! running->events.first) {
          printf("\t\tend process [%d]\n", running->pid);
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
        cpu->running = 0;
      }
    }


    // if running not defined and ready queue not empty
    // put the first in ready to run
    // if (cpu->status == IDLE && os->ready.first) {
    //   FakePCB* pcb = (FakePCB*) List_popFront(&os->ready);
    //   FakeCPU_assign_process(cpu, pcb);
    // }
    
    aux = aux->next;
  }
  // TODO : gestire la lista di processi in attesa di risorse con MULTICORE

  // call schedule, if defined
  if (os->schedule_fn) {
    (*os->schedule_fn)(os, os->schedule_args);
  }

  ++os->timer;

}


int FakeCPU_init(FakeOS* os, int n_cpus) {
    int loaded = 0;
    for(int i = 0; i < n_cpus; i++) {
        FakeCPU new_cpu;
        new_cpu.id = i;
        new_cpu.status = IDLE;
        new_cpu.running = NULL;

        FakeCPU* cpu_ptr = (FakeCPU*)malloc(sizeof(FakeCPU));
        *cpu_ptr = new_cpu;
        cpu_ptr->list.next = cpu_ptr->list.prev = 0;
        if (List_pushBack(&os->cpus, (ListItem*) cpu_ptr)) {
          loaded++;
        }
    }
    os->num_cpus = loaded;
    return loaded;
}

FakeCPU* FakeCPU_find_idle(FakeOS* os) {

    ListItem* aux = (ListItem*) os->cpus.first;
    while(aux) {
        FakeCPU* cpu = (FakeCPU*)aux;
        if(cpu->status == IDLE) {
            return cpu;
        }
        aux = aux->next;
    }
    return 0;
}

int FakeCPU_assign_process(FakeCPU* cpu, FakePCB* pcb)  {
    assert(cpu->status == IDLE); // come altrimenti? :D
    // TODO: verificare che os non abbia già una CPU con lo stesso processo in esecuzione?
    cpu->running = pcb;
    cpu->status = BUSY;
    return 1;
}

int FakeCPU_to_idle(FakeCPU* cpu) {
    assert(cpu->status == BUSY);
    cpu->running = NULL;
    cpu->status = IDLE;
    return 1;
}

void FakeOS_destroy_cpus(FakeOS* os) {

  // Deallochiamo la memoria utilizzata per le istanze cpu
  ListItem* aux = os->cpus.first;
  printf("\nDeallocating all cpus...");
  while(aux) {
    FakeCPU* cpu = (FakeCPU*) aux;
    aux = aux->next;
    free(cpu);

  }
  printf("DONE.");
}

int FakeOS_check_all_idle(FakeOS* os) {
  ListItem* aux = (ListItem*) os->cpus.first;
  while(aux) {
    FakeCPU* cpu = (FakeCPU*) aux;

    if(cpu->status == BUSY) {
      return 0;
    }

    aux = aux->next;
  }
  return 1;
}

void FakeOS_print_cpu(FakeOS* os) {
  ListItem* aux = os->cpus.first;
  while(aux) {
    FakeCPU* cpu = (FakeCPU*) aux;
    printf("CPU id:%d, status:%d\n", cpu->id, cpu->status);
    aux = aux->next;
  }
}