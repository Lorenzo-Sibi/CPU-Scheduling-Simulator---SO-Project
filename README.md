# Fake OS - Operating System Simulator

## Overview

Fake OS is a simple operating system simulator written in C. The primary goal of this project is to provide a testing environment for process scheduling algorithms in a multicore simulated environment.

## Components

The project consists of several source files:

- `fake_os.c` and `fake_os.h`: Implement core functionalities of the operating system, including initialization, process creation and execution, and CPU management, the latter result in particular is implemented using the FakeCPU data structure.
- `fake_process.c` and `fake_process.h`: Define the structure and functions related to simulated processes, including loading and saving processes from and to files.
- `linked_list.c` and `linked_list.h`: Implement a generic linked list used for managing process queues and other lists within the operating system.
- `sched_sim.c`: Contains the main function `main` that initializes and runs the operating system simulation, including process creation and definition of the scheduling algorithm to be used.

## Compilation and Execution

To compile the project, ensure you have a C compiler installed on your system. Then, run the `make all` command from the terminal. This will generate the `sched_sim` executable.

To execute the program, you can use the command `./sched_sim <number_of_cpus> <process_file_1.txt> <process_file_2.txt> ...` or (if you have multiple files with similar name in the same folder i.e. p1.txt, p2.txt,...) `./sched_sim <number_of_cpus> *.txt>`.
The number of CPUs indicates how many processors will be simulated, while the process files contain information about the processes to be loaded into the simulation.

## Scheduling Algorithms

The program currently supports two scheduling algorithms:

1. **Round Robin (RR)**: A time-sharing scheduling algorithm that assigns a fixed time quantum to each process.
2. **Shortest Job First with Quantum Prediction (SJFP)**: A priority-based scheduling algorithm that assigns the CPU to the process with the shortest predicted execution time. It uses a burst prediction formula: `q(t+1) = a * q_current + (1-a) * q(t)`.

You can select the desired scheduling algorithm by modifying the `sched_sim.c` file and setting the appropriate scheduling function in the `main` function.

## How It Works

1. **Initialization**: The Fake OS initializes the system components, including the CPU, process queues, and other data structures.
   
2. **Process Loading**: Processes are loaded from input files, each containing information about process IDs, arrival times, and bursts.
   
3. **Scheduling**: Based on the selected scheduling algorithm, the OS assigns processes to available CPUs for execution.
   
4. **Simulation Step**: The OS simulates the execution of processes by incrementing a timer and handling process events such as CPU bursts and I/O operations.
   
5. **Process Termination**: Processes are terminated when their execution is completed, and resources are deallocated.
   
6. **End of Simulation**: The simulation ends when all processes are completed, and the CPUs are idle.

## Notes

This project is based on an exercise from the course "Sistemi Operativi" (Operating Systems) at Università degli Studi di Roma "La Sapienza". Only certain parts of the code have been written by the author. The original codebase can be found at [https://gitlab.com/grisetti/sistemi_operativi_2022_23](https://gitlab.com/grisetti/sistemi_operativi_2022_23).

Feel free to contribute to the project or use it for educational or study purposes. If you find bugs or have suggestions for improving the code, open an issue or submit a pull request! :D
