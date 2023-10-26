# Lab 2 Part 2 Design Doc: Inter-process Communication

## Overview

The goal of this lab is to enable inter-process communication by implementing the `pipe` system call and to execute user programs by adding the `exec` system call.
### Major Parts

#### pipe
Create a pipe, a kernel buffer that is shared between processes. One process writes to the pipe, and the other reads from it. Returns two file descriptors, one for reading and one for writing.


#### exec
Execute a user program. The program is loaded into the virtual address space of the calling process, and the process is replaced by the new program. The new program is executed with the arguments passed in.


## In-depth Analysis and Implementation

### Functions

- `pipe`: Create a pipe and two file descriptors for reading and writing.
  - Allocate a (4KB) page for the pipe's buffer and metadata using `kalloc`.
  - Open a pipe with the parent, then fork. The child process inherits the file descriptors of the parent.
  - Close the parent's reading file descriptor and the child's writing file descriptor.
  - Returns two file descriptors, one for reading and one for writing.

- `exec`: Execute an user program on the existing process.
  - Create a new temporary address space using `vspaceinit`.
  - Read and load the program using `vspaceloadcode`.
  - Setup and initialize the user stack using `vspaceinitstack`.
  - Copy the arguments from the current address space to the new one using `vspacewritetova`.
  - Copy and overwrite the current address space using `vspacecopy`.
  - Free the temporary address space at the end or on failure to avoid memory leaks using `vspacefree`.
  - Return 0 on success, -1 on failure.

- `pipe_read`:

- `pipe_write`:


### System Calls
#### sys_pipe, sys_exec
The main goals of the `sys_*` functions is to do argument parsing and then calling the associated functions.

#### kalloc.c
We will need `kalloc` to allocate a page for the pipe buffer.

#### vspace.c
We will need:
- `vspaceinit` to initialize the virtual address space of the new process.
- `vspaceloadcode` to read the program and load it into the passed in address space.
- `vspaceinitstack` to setup and initialize the user stack.
- `vspacewritetova` to create a deep copy of the argument from the old address space to the new one.
- `vspacecopy` to copy the current address space to the new process.
- `vspacefree` to free the old address space to avoid memory leaks.


## Risk Analysis

### Unanswered Questions



### Staging of Work



### Time Estimation
Best / Average / Worst case scenario
- Functions
  - pipe (2/4/6 hours)
  - exec (4/6/8 hours)
