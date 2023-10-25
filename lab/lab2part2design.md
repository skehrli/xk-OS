# Lab 2 Part 2 Design Doc: Inter-process Communication

## Overview

The goal of this lab is to enable inter-process communication by implementing the `pipe` system call and to execute user programs by adding the `exec` system call.
### Major Parts

#### pipe
Create a pipe, a kernel buffer that is shared between processes. One process writes to the pipe, and the other reads from it. Returns two file descriptors, one for reading and one for writing.


#### exec
Execute a user program. The program is loaded into the virtual address space of the calling process, and the process is replaced by the new program. The new program will start executing at the entry point specified in the ELF header.


## In-depth Analysis and Implementation

### Functions

- `sys_pipe`: 

- `sys_exec`:

- `exec`:


#### vspace.c
We will need `vspacewritetova`



## Risk Analysis

### Unanswered Questions



### Staging of Work



### Time Estimation
Best / Average / Worst case scenario
- Functions
  - pipe (2/4/6 hours)
  - exec (4/6/8 hours)
