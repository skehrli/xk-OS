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

- Use spinlocks for global file table
- Acquire lock before accessing the `file_info` and release it after

### Functions

- `fork`: Create a new process as a copy of the calling process. Returns twice, once in the parent, with the return value of the pid of the child, and once in the child, with the return value of 0.
  - Use `allocproc` to create a new entry in the process table
  - Use `vspacecopy` to copy the parent's virtual address space to the child
  - Duplicate the file descriptors in the new process
  - Duplicate the trap frame in the new process
  - Set the state of the new process to `RUNNABLE`
- `wait`: Sleep until a child process terminates, then return child’s pid.
  - Look for exited children in the process table
  - Wait for child process to call `exit`
  - Clean the child process (vspace with `vspacefree` and kernel stack with `kfree`)

- `exit`: Terminate the calling process, and return the exit status to the parent.
  - Close all open file descriptors.
  - Mark the process as `ZOMBIE`
  - Wake up the parent process
  - Does not return, but calls `sched` to reschedule the next process


#### vspace.c
We will need `vspaceinit` and `vspacecopy` to initialize and copy the virtual address space when creating a new process.



## Risk Analysis

### Unanswered Questions

- How to select the appropriate lock granularity? 

### Staging of Work
First the functions will be implemented and then we will define the critical sections and implement the synchronization using locks.

### Time Estimation
Best / Average / Worst case scenario
- Synchronization
  - Design (2 hours)
  - Implementation (2/6/10 hours)
- Functions
  - fork (2/4/6 hours)
  - wait (2/3/4 hours)
  - exit (2/3/4 hours)
- Edge cases and Error handling (2 hours)
