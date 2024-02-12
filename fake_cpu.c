#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "fake_cpu.h"

int FakeCPU_init(FakeOS* os, int n_cpus) {
    for(int i = 0; i < n_cpus; i++) {
        FakeCPU new_cpu;
        new_cpu.id = i;
        new_cpu.os = os;
        new_cpu.status = IDLE;
        new_cpu.running = NULL;

        FakeCPU * cpu_ptr = (FakeCPU*)malloc(sizeof(FakeCPU));
        *cpu_ptr = new_cpu;
        List_pushBack(&os.cpus, (ListItem*) cpu_ptr);
    }
    os->n_cpus = n_cpus;
    return n_cpus;
}

FakeCPU_assign_process(FakeCPU* cpu, FakePCB* pcb)  {
    assert(cpu->status == IDLE); // come altrimenti? :D
    // TODO: verificare che os non abbia già una CPU con lo stesso processo in esecuzione?
    cpu->running = pcb;
    cpu->status = BUSY;
    return 1;
}

FakeCPU_preempt_process(FakeCPU* cpu, FakePCB* pcb) {
    assert(cpu-status == BUSY);
    cpu->running = NULL;
    cpu->status = IDLE;
    return 1;
}

FakeCPU_find_idle(FakeOS* os) {

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
