#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "fake_os.h"

#ifndef DEFAULT_QUANTUM
#define DEFAULT_QUANTUM 10 /* in ms */
#endif

#ifndef DEFAULT_ALPHA
#define DEFAULT_ALPHA 0.6
#endif

#ifdef _LIST_DEBUG_
#define MAX_STEPS 10000
#endif

FakeOS os;

typedef struct {
  int quantum;
} SchedRRArgs;

typedef struct {
  float alpha;
} SchedSJFPArgs;

void schedRR(FakeOS* os, void* args_){
  SchedRRArgs* args=(SchedRRArgs*)args_; // quantum

  // look for the first process in ready
  // if none, return
  if (!os->ready.first)
    return;

  FakePCB* pcb=(FakePCB*) List_popFront(&os->ready);
  FakeCPU* cpu_idle = FakeCPU_find_idle(os);
  
  if(!cpu_idle) return;
  
  FakeCPU_assign_process(cpu_idle, pcb);
  
  assert(pcb->events.first);
  ProcessEvent* e = (ProcessEvent*)pcb->events.first;
  assert(e->type==CPU);

  // look at the first event
  // if duration>quantum
  // push front in the list of event a CPU event of duration quantum
  // alter the duration of the old event subtracting quantum
  if (e->duration>args->quantum) {
    ProcessEvent* qe=(ProcessEvent*)malloc(sizeof(ProcessEvent));
    qe->list.prev=qe->list.next=0;
    qe->type=CPU;
    qe->duration=args->quantum;
    e->duration-=args->quantum;
    List_pushFront(&pcb->events, (ListItem*)qe);
  }
};

void update_predicted_burst(FakeOS* os, float alpha) {
  ListItem* aux_r = os->ready.first;
  ListItem* aux_w = os->waiting.first;
  while(aux_r || aux_w) {
    FakePCB* pcb_r = aux_r ? (FakePCB*) aux_r : 0;
    FakePCB* pcb_w = aux_w ? (FakePCB*) aux_w : 0;
    if(pcb_r && pcb_r->update_prediction) {
      pcb_r->predicted_burst = alpha * pcb_r->running_time + (1 - alpha) * pcb_r->predicted_burst;
      pcb_r->update_prediction = 0;
      pcb_r->running_time = 0;
      printf("\t\tReady process %d: predicted burst updated to %d\n", pcb_r->pid, pcb_r->predicted_burst);
    }
    if(pcb_w && pcb_w->update_prediction) {
      pcb_w->predicted_burst = alpha * pcb_w->running_time + (1 - alpha) * pcb_w->predicted_burst;
      pcb_w->update_prediction = 0;
      pcb_w->running_time = 0;
      printf("\t\tWaiting process %d: predicted burst updated to %d\n", pcb_w->pid, pcb_w->predicted_burst);
    }

    if(aux_r) aux_r = aux_r->next;
    if(aux_w) aux_w = aux_w->next;
  }
}

void check_terminated_quant(FakeOS* os) {
  ListItem* aux = os->cpus.first;
  while (aux) {
    FakeCPU* cpu = (FakeCPU*) aux;
    FakePCB* running = cpu->running;
    if(running) {
      ProcessEvent* current_event = (ProcessEvent*) running->events.first;
      assert(current_event->type == CPU);
      // al prosimo step il processo verrà messo in waiting o in ready, ad ogni modo il suo quant è terminato e va aggiornato
      if(current_event->duration == 1) running->update_prediction = 1;
    }

    aux = aux->next;
  }
  
}

void schedSJFP(FakeOS* os, void* args_) {
  // TODO: migliorare descrizione funzione?
  // Implementa la logica dello scheduling preemptive shortest job first con previsione del quantum
  // Formula per la previsione del quantum: q(t+1) = a * q_current + (1-a) * q(t)
  SchedSJFPArgs* args = (SchedSJFPArgs*)args_;
  float alpha = args->alpha;
  FakePCB* shortest_job = NULL;

  check_terminated_quant(os);
  update_predicted_burst(os, alpha);

  ListItem* aux = os->ready.first;
  while (aux) {
    FakePCB* pcb = (FakePCB*)aux;
    if (!shortest_job || pcb->predicted_burst < shortest_job->predicted_burst) {
      shortest_job = pcb;
    }
    aux = aux->next;
  }

  FakeCPU* cpu_idle = FakeCPU_find_idle(os);
  if (shortest_job && cpu_idle) {
    List_detach(&os->ready, (ListItem*)shortest_job);
    FakeCPU_assign_process(cpu_idle, shortest_job);

    ProcessEvent* e = (ProcessEvent*)shortest_job->events.first;
    assert(e->type == CPU);
    if(e->duration > shortest_job->predicted_burst) {
      ProcessEvent* qu_e = (ProcessEvent*)malloc(sizeof(ProcessEvent));
      qu_e->list.prev = qu_e->list.next = 0;
      qu_e->type = CPU; // assert fatta!
      qu_e->duration = shortest_job->predicted_burst;
      e->duration -= shortest_job->predicted_burst;
      List_pushFront(&shortest_job->events, (ListItem*)qu_e);
    }
    //printf("Process %d assigned to CPU with shortest CPU burst. New quant: %d\n", shortest_job->pid, new_duration);
  }
}

int main(int argc, char** argv) {

  if (argc < 3) {
    printf("Usage: %s <number_of_cpus> <process_file_1.txt> <process_file_2.txt> ...\n", argv[0]);
    return -1;
  }

  // Inizializzazione OS
  FakeOS_init(&os);
  SchedRRArgs srr_args;
  srr_args.quantum=DEFAULT_QUANTUM;
  SchedSJFPArgs sjfp_args;
  sjfp_args.alpha=DEFAULT_ALPHA;

  // Settaggio algoritmo di scheduling
  os.schedule_args=&sjfp_args;
  os.schedule_fn=schedSJFP;

  printf("OS initialized.\n");

  // Inizializzazione CPUS
  int n_cpus = atoi(argv[1]);
  n_cpus = FakeCPU_init(&os, n_cpus);
  assert(n_cpus==os.num_cpus);
  printf("Creating %d cpu's instances...\n", n_cpus);
  FakeOS_print_cpu(&os);

  // Inizializzazione Processi
  for (int i=2; i<argc; ++i){
    FakeProcess new_process;
    int num_events=FakeProcess_load(&new_process, argv[i]);
    printf("loading process [%s], pid: %d, events:%d\n",
           argv[i], new_process.pid, num_events);
    if (num_events) {
      FakeProcess* new_process_ptr=(FakeProcess*)malloc(sizeof(FakeProcess));
      *new_process_ptr=new_process;
      List_pushBack(&os.processes, (ListItem*)new_process_ptr);
    }
  }
  printf("num processes in queue %d\n", os.processes.size);


  int steps = 0;
  while(!FakeOS_check_all_idle(&os)
        || os.ready.first
        || os.waiting.first
        || os.processes.first){
    if(steps++>MAX_STEPS) {
      printf("************* MAX STEPS REACHED ************\n");
      break;
    }
    FakeOS_simStep(&os);
  }
  printf("************** SIMULATION ENDED **************\n");
  FakeOS_destroy_cpus(&os);
}
