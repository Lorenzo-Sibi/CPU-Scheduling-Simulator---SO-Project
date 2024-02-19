#pragma once
#include "linked_list.h"

typedef enum {CPU=0, IO=1} ResourceType;

// event of a process, is in a list
typedef struct {
  ListItem list;
  ResourceType type;
  int duration;
} ProcessEvent;

// fake process
typedef struct {
  ListItem list;
  int pid; // assigned by us
  int arrival_time;
  ListHead events;
} FakeProcess;

typedef struct {
  ListItem list;
  ListHead events;
  int pid;
  int predicted_burst;
  short int update_prediction;
} FakePCB;

int FakeProcess_load(FakeProcess* p, const char* filename);

int FakeProcess_save(const FakeProcess* p, const char* filename);
