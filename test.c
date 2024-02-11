#include <stdio.h>
#include <stdlib.h>

#include "fake_cpu.h"
#include "fake_os.h"
#include "fake_process.h"

FakeOS os;

int main(int argc, char** argv) {
    FakeOS_init(&os);

    int n_cpus = FakeOS_cpu_init(&os, "cpus.txt");
    FakeOS_print_cpu(&os);
    printf("%d TOTALS CPU\n", n_cpus);
    return 0;
}