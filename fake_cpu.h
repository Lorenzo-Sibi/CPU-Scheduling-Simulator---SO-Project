#include "fake_os.h"
#include "linked_list.h"
#pragma once

typedef enum {BUSY=0, IDLE=1} CPUStatus;

// Ogni processore ha il proprio scheduling e dunque il proprio PCB attualmente in modalità running
// Ogni CPU condivide la stessa coda ready e coda waiting con gli altri processori (sharing queues architecture)
typedef struct {
    FakePCB* running;
    ScheduleFn schedule_fn;
    int cpu_id;
    CPUStatus status;
    void* schedule_args;
} FakeCPU;

// Essendo le code condivise sono necessari meccanismi di mutua esclusione (es. semafori)

void FakeCPU_init(FakeCPU* cpu);