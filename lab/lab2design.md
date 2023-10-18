# Lab 2 Part 1 Design Doc: Enabling Multiprocessing

## Overview

The goal of this lab is to enable multiprocessing by implementing functions to start/stop a new process and to handle synchronization between files using locks.

### Major Parts

#### Synchronization issues
Need to handle multiples processes accessing the same data. We will use locks for mutual exclusion.

Spinlock vs Sleeplock

#### fork
Create a new process as a copy of the calling process. Returns twice, once in the parent, with the return value of the process ID (pid) of the child, and once in the child, with the return value of 0.

#### wait/exit
System calls to synchronize parent and child processes. `wait` will block until the child process exits, and then return the child's exit status. `exit` will terminate the calling process, and return the exit status to the parent.


## In-depth Analysis and Implementation


## Risk Analysis

