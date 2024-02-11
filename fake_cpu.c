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