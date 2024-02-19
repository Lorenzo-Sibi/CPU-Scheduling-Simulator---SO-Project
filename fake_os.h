#include "linked_list.h"
#include "fake_process.h"

struct FakeOS;
struct FakeCPU;

typedef void (*ScheduleFn)(struct FakeOS* os, void* args);
typedef enum {IDLE=0, BUSY=1} CPUStatus;

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

/*-----------FakeCPU------------*/
// Ogni processore ha il proprio scheduling e dunque il proprio PCB attualmente in modalità running
// Ogni CPU condivide la stessa coda ready e coda waiting con gli altri processori (sharing queues architecture)

typedef struct FakeCPU{
    ListItem list;
    int id;
    CPUStatus status;
    FakePCB* running;  // Processo attualmente in esecuzione...
} FakeCPU;

void FakeOS_init(FakeOS* os);
void FakeOS_simStep(FakeOS* os);
void FakeOS_destroy(FakeOS* os);

int FakeOS_check_all_idle(FakeOS* os);
int FakeOS_cpu_init(FakeOS* os, const char* filename);
void FakeOS_destroy_cpus(FakeOS* os);
void FakeOS_print_cpu(FakeOS* os);


int FakeCPU_init(FakeOS *os, int n_cpus);
FakeCPU* FakeCPU_find_idle(FakeOS* os);
int FakeCPU_to_idle(FakeCPU* cpu);
int FakeCPU_assign_process(FakeCPU* cpu, FakePCB* pcb);