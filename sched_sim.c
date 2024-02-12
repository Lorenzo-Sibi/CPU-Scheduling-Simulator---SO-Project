#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "fake_os.h"
#include "fake_cpu.h"

#ifndef DEFAULT_QUANTUM
#define DEFAULT_QUANTUM 10 /* in ms */
#endif

FakeOS os;

typedef struct {
  int quantum;
} SchedRRArgs;

typedef struct {
  float alpha;
} SchedSJFP;

void schedRR(FakeOS* os, void* args_){
  SchedRRArgs* args=(SchedRRArgs*)args_;

  // look for the first process in ready
  // if none, return
  if (! os->ready.first)
    return;

  FakePCB* pcb=(FakePCB*) List_popFront(&os->ready);
  os->running=pcb;
  
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

void schedSJFP(FakeOS* os, void* args) {
    // TODO: migliorare descrizione funzione?
    // Implementa la logica dello scheduling preemptive shortest job first con previsione del quantum
    // Formula per la previsione del quantum: q(t+1) = a * q_current + (1-a) * q(t)

    SchedSJFP* alpha = *(SchedSJFP*)args;
    FakePCB* shortest_job = NULL;

    // Funzione AUSILIARIA? -> Valutare :)
    ListItem* aux = os->ready.first;
    while (aux) {
        FakePCB* pcb = (FakePCB*)aux;
        ProcessEvent* cpu_event = (ProcessEvent*)pcb->events.first;
        if (!shortest_job || cpu_event->duration < ((ProcessEvent*)shortest_job->events.first)->duration) {
            shortest_job = pcb;
        }
        aux = aux->next;
    }

    if (shortest_job) {
        List_detach(&os->ready, (ListItem*)shortest_job);
        os->running = shortest_job;

        // Ora calcola il nuovo quantum
        int old_duration = ((ProcessEvent*)shortest_job->events.first)->duration; // TODO: rendi più legeibile questa riga
        int new_duration = alpha * old_duration + (1 - alpha) * os->quantum_prediction;
        ((ProcessEvent*)shortest_job->events.first)->duration = new_duration;

        printf("Process %d assigned to CPU with shortest CPU burst. New quant: %d\n", shortest_job->pid, new_duration);
    }
}


int main(int argc, char** argv) {

  // Inizializzazione OS
  FakeOS_init(&os);
  SchedRRArgs srr_args;
  srr_args.quantum=5;
  os.schedule_args=&srr_args;
  os.schedule_fn=schedRR;

  // Inizializzazione CPUS
  int n_cpus = atoi(argv[1]);
  n_cpus = FakeCPU_init(&os, n_cpus);
  assert(n_cpus==os.num_cpus);
  printf("Creating %d cpu's instances\n", n_cpus);

  // Inizializzazione Processi
  for (int i=2; i<argc; ++i){
    FakeProcess new_process;
    int num_events=FakeProcess_load(&new_process, argv[i]);
    printf("loading [%s], pid: %d, events:%d",
           argv[i], new_process.pid, num_events);
    if (num_events) {
      FakeProcess* new_process_ptr=(FakeProcess*)malloc(sizeof(FakeProcess));
      *new_process_ptr=new_process;
      List_pushBack(&os.processes, (ListItem*)new_process_ptr);
    }
  }
  printf("num processes in queue %d\n", os.processes.size);
  while(os.running
        || os.ready.first
        || os.waiting.first
        || os.processes.first){
    FakeOS_simStep(&os);
  }
}
