#include "fake_process.h"
#include "linked_list.h"
#pragma once


typedef struct {
  ListItem list;
  int pid;
  ListHead events;
} FakePCB;

struct FakeOS;
typedef void (*ScheduleFn)(struct FakeOS* os, void* args);

typedef struct FakeOS{
  FakePCB* running;
  ListHead ready;
  ListHead waiting;
  ListHead cpus;
  ListHead processes;
  int timer;
  ScheduleFn schedule_fn;
  void* schedule_args;
} FakeOS;

void FakeOS_init(FakeOS* os);
void FakeOS_simStep(FakeOS* os);
void FakeOS_destroy(FakeOS* os);

int FakeOS_cpu_init(FakeOS* os, const char* filename);
void FakeOS_print_cpu(FakeOS* os);