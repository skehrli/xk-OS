# Lab 2 Part 1 Design Doc: Enabling Multiprocessing

## Overview

The goal of this lab is to enable multiprocessing by implementing functions to create/exit a process and to handle synchronization between files using locks.

### Major Parts

#### Synchronization issues
Need to handle multiples processes accessing the same data. We will use locks for mutual exclusion.

Spinlock vs Sleeplock (global file table, process file info, granularity?).

#### fork
Create a new process as a copy of the calling process. Returns twice, once in the parent, with the return value of the process ID (pid) of the child, and once in the child, with the return value of 0.

#### wait/exit
System calls to synchronize parent and child processes. `wait` will block until the child process exits, and then return the child's exit status. `exit` will terminate the calling process, and return the exit status to the parent.


## In-depth Analysis and Implementation

### Synchronization
We need to handle multiple processes accessing the global file table `file_table`. We will use locks to ensure mutual exclusion.

- Spinlocks are suitable for protecting small critical sections where the expected wait times are short. The disadvantage is that they waste CPU cycles while waiting for the lock to become available.
- Sleeplocks are more appropriate for protecting larger critical sections. This avoids wasting CPU cycles but may introduce additional context switching overhead.

We will use spinlocks for the system calls that access the `file_info` because the expected wait times are short and sleeplocks are already used to interact with the inode interface with `locki` and `unlocki` in fs.c

### Functions

- `fork`: Create a new process as a copy of the calling process. Returns twice, once in the parent, with the return value of the process ID (pid) of the child, and once in the child, with the return value of 0.
- `wait`: Block until the child process exits, and then return the child's exit status.
- `exit`: Terminate the calling process, and return the exit status to the parent.

#### vspace.c
We will need `vspaceinit` and `vspacecopy` to initialize and copy the virtual address space when creating a new process.



## Risk Analysis

### Unanswered Questions

- How to select the appropriate lock granularity? 

### Staging of Work
First the functions will be implemented and then we will define the critical sections and implement the synchronization using locks.

### Time Estimation
- Synchronization
  - Design (2 hours)
  - Implementation (2/4/6 hours)
- Functions
  - fork (2/3/4 hours)
  - wait (2/3/4 hours)
  - exit (2/3/4 hours)
- Edge cases and Error handling (2 hours)
