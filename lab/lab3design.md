# Lab 3 Design Doc: Address Space Management

## Overview

The goal is to optimize memory usage by focusing on address space management.We will be implementing a user-level heap, initializing the shell, growing the user stack on-demand, and implementing copy-on-write fork.


### Major Parts

#### Part 1: Create a user-level heap

A user-level heap allows processes to allocate and deallocate memory at runtime. We will implement the `sbrk` system call, which increases the size of the heap by `n` bytes. It returns the previous end of the heap on success, and -1 on failure.

#### Part 2: Starting shell
The shell will be loaded from disk, and the system will boot into the shell. `user/init.c` will fork into two processes. One will load `user/sh.c`, the other
will wait for zombie processes to reap. This involves changing the line in `kernel/initcode.S`:
```
init:
  .string "/init\0"
```

#### Part 3: Grow user stack on-demand
We will optimize memory consumption by implementing on-demand stack growth. Instead of allocating the entire user stack at the beginning, we will allocate memory as needed. This requires handling page faults and dynamically allocate stack pages as they are needed.

#### Part 4: Copy-on-write fork
To improve the performance of the fork operation, we will implement a copy-on-write mechanism. Instead of duplicating every page of user memory for a child process, we will allow multiple processes to share the same physical memory while maintaining logical separation. Pages will be copied only when a write operation occurs.


## In-depth Analysis and Implementation

### Functions

- `sbrk`:
    - Increase the size of the heap by `n` bytes.
    - Use `vregionaddmap` to add a new mapping to the heap region.
    - Use `vspaceupdate` to update the actual page table entries to reflect the newly added mappings.
    - Returns the previous end of the heap on success, and -1 on failure.
- `trap`:
    - Implement on-demand stack growth by handling page faults.
    - Define function `grow_ustack` that allocates a new page for the user stack, which is similar to `sbrk`.
    - If `grow_ustack` is successful, return to the instruction to resume execution, otherwise let the kernel handle the page fault.
- `grow_ustack`:
    - Increase the size of the user stack by one page `PGSIZE`.
    - Use `vregionaddmap` to add a new mapping to the stack region.
    - Use `vspaceupdate` to update the actual page table entries to reflect the newly added mappings.
    - Returns the previous top of the stack on success, and -1 on failure.

### System Calls
#### sys_sbrk
The main goals of the `sys_*` functions is to do argument parsing and then calling the associated functions.


## Risk Analysis

### Unanswered Questions


### Staging of Work


### Time Estimation
Best / Average / Worst case scenario
- Functions:
    - `sbrk`: (2/4/6 hours)
    - `trap`: (2/4/6 hours)
    - `cow_fork`: (4/6/8 hours)
