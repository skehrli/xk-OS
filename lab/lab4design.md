# Lab 4 Design Doc: Filesystem

## Overview

The goal of this lab is to implement the filesystem interface for users to interact with persistent media.

### Major Parts

#### Part A: Enable Writes and Deletes

In this part, we aim to enable write functionality in the file system. This involves changing the disk layout to accommodate file extensions. The file operations, specifically the open system call in `file.c`, and `sysfile.c`, need to be modified to support write operations. Additionally, modifications in `kernel/fs.c` are required to implement file writes on the disk. The use of `bread`, `bwrite`, and `brelse` functions will be crucial.

#### Part B: Support Concurrent Filesys Operations

To ensure the writable file system supports multiprocessing and handles concurrency issues, synchronization needs to be added. This involves using `locki(ip)` and `unlocki(ip)` at appropriate places in the code. Care must be taken when holding multiple locks to avoid deadlocks, and proper lock acquisition order should be maintained. Tests in lab4test_b.c should pass when the code is run from the shell to confirm synchronization support.

#### Part C: Crash Safety

The goal in this part is to make the file system crash-safe by implementing journaling. Journaling involves writing each block of a multi-block operation to a separate area of the disk (the log) before committing changes to the main part of the disk. Two transactions, file write and file create, need to be implemented using helper functions begin_tx() and commit_tx(). A wrapper function log_write is necessary to write to the log region, and proper handling of dirty blocks is required to prevent eviction by the block cache. Tests in lab4test_c.c will evaluate the crash safety of the implemented file system.

## In-depth Analysis and Implementation



#### Functions

- `iopen`


## Risk Analysis

### Unanswered Questions

### Staging of Work

### Time Estimation

