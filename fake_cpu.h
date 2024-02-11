#include "fake_os.h"
#include "linked_list.h"
#pragma once

typedef enum {IDLE=0, BUSY=1} CPUStatus;
typedef enum {NONE = -0x01, RR = 0x00, PSJF = 0x01} SchedulingAlgorithm;

// Ogni processore ha il proprio scheduling e dunque il proprio PCB attualmente in modalità running
// Ogni CPU condivide la stessa coda ready e coda waiting con gli altri processori (sharing queues architecture)
typedef struct {
    ListItem list;
    int id;
    CPUStatus status;
    FakePCB* running;  // Processo attualmente in esecuzione...
} FakeCPU;

int FakeCPU_init(FakeOS *os, int n_cpus);