# Lab 3 Design Doc: Address Space Management

## Overview

The goal is to optimize memory usage by focusing on address space management.We will be implementing a user-level heap, initializing the shell, growing the user stack on-demand, and implementing copy-on-write fork.


### Major Parts

#### Part 1: Create a user-level heap

A user-level heap allows processes to allocate and deallocate memory at runtime. We will implement the `sbrk` system call, which increases the size of the heap by `n` bytes. It returns the previous end of the heap on success, and -1 on failure.

#### Part 2: Starting shell
The shell will be loaded from disk, and the system will boot into the shell. This involves modifying the initialization code and setting up user processes, allowing users to run commands within the shell.

#### Part 3: Grow user stack on-demand
We will optimize memory consumption by implementing on-demand stack growth. Instead of allocating the entire user stack at the beginning, we will allocate memory as needed. This requires handling page faults and dynamically allocate stack pages as they are needed.

#### Part 4: Copy-on-write fork
To improve the performance of the fork operation, we will implement a copy-on-write mechanism. Instead of duplicating every page of user memory for a child process, we will allow multiple processes to share the same physical memory while maintaining logical separation. Pages will be copied only when a write operation occurs.


## In-depth Analysis and Implementation

### Functions


## Risk Analysis

### Unanswered Questions


### Staging of Work


### Time Estimation
Best / Average / Worst case scenario
