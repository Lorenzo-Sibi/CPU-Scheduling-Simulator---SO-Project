#include "fake_process.h"
#include "fake_cpu.h"
#include "linked_list.h"
#pragma once


typedef struct {
  ListItem list;
  ListHead events;
  int pid;
  int burst_time;
  int predicted_burst;
} FakePCB;

struct FakeOS;
typedef void (*ScheduleFn)(struct FakeOS* os, void* args);

typedef struct FakeOS{
  ListHead ready;
  ListHead waiting;
  ListHead cpus;
  ListHead processes;
  int num_cpus;
  int timer;
  ScheduleFn schedule_fn;
  void* schedule_args;
} FakeOS;

void FakeOS_init(FakeOS* os);
void FakeOS_simStep(FakeOS* os);
void FakeOS_destroy(FakeOS* os);

int FakeOS_cpu_init(FakeOS* os, const char* filename);
int FakeOS_cpu_destroy(FakeOS* os, *FakeCPU cpu);
void FakeOS_print_cpu(FakeOS* os);