# Lab 4 Design Doc: Filesystem

## Overview

The goal of this lab is to implement the filesystem interface for users to interact with persistent media. We will enable write functionality, support concurrent filesystem operations, and implement crash safety measures through journaling.

### Major Parts

#### Part A: Enable Writes and Deletes

In this part, we aim to enable write functionality in the file system. This involves changing the disk layout to accommodate file extensions. The file operations, specifically the open system call in `file.c`, and `sysfile.c`, need to be modified to support write operations. Additionally, modifications in `kernel/fs.c` are required to implement file writes on the disk. The use of `bread`, `bwrite`, and `brelse` functions will be crucial.

#### Part B: Support Concurrent Filesys Operations

To ensure the writable file system supports multiprocessing and handles concurrency issues, synchronization needs to be added. This involves using `locki(ip)` and `unlocki(ip)` at appropriate places in the code. Care must be taken when holding multiple locks to avoid deadlocks, and proper lock acquisition order should be maintained.

#### Part C: Crash Safety

The goal in this part is to make the file system crash-safe by implementing journaling. Journaling involves writing each block of a multi-block operation to a separate area of the disk (the log) before committing changes to the main part of the disk. We will use helper functions `begin_tx()` and `commit_tx()`. 

## In-depth Analysis and Implementation

### Functions

#### Modification
- `file_open`
    
    Remove Write Restriction:
    - Allow opening files in write mode.

    Support Overwrite:
    - Enable overwriting data in existing blocks.
    - Ensure file size (metadata) remains unchanged after overwrite.

    Support Append:
    - Implement writing data past the end of the file.
    - Handle metadata changes, potentially leading to additional block allocation.

    Create new file:
    - Allocate a new dinode in the inodefile.
    - Update data of the root directory to track a new dirent.

- `writei`
    - Responsible for performing updates to data and metadata (inode)
    - Utilizes functions to interact with the block cache `(bget, bwrite, brelse)`.
    - Can only read/write in units of blocks/sectors
    - Use `write_file` when overwriting data in existing blocks.
    - Use `append_file` when writing data past the end of the file.

#### Implementation
- `sys_unlink`

    Deletion of Files
    - Check if the file exists and has no open references.
    - Free the dinode associated with the file.
    - Ensure file creation can fill holes in the inodefile.
    - Update parent directory's dirents to reflect the deletion.

- `create_file`

    Create a new file when `O_CREATE` is passed to `file_open`
    - Check if the file exists.
    - Allocate a new dinode in the inodefile. Check if there are any free dinode in the inodefile, otherwise append a new dinode to the inodefile.
    - Update data of the root directory to track a new dirent.

- `write_file`

    Overwrite data in existing data blocks
    - file size (metadata) is not changed after an overwrite

- `append_file`

    Write data past the end of the file
    - Allocates additional blocks if needed (using bitmap functions to find free blocks).
    - Change file size (metadata) to reflect the new size of the file.
    - Modifies data layout to allow tracking multiple extents.

### Synchronization

Since the file system is now writable, we need to handle multiple processes and ensure synchronization.

- Use `locki(ip)` and `unlocki(ip)` to protect the inode layer when accessing the file system.
- Ensure proper lock acquisition order to avoid deadlocks.

We will need to modify the following functions in `file.c` to ensure synchronization:
- `file_read`
- `file_write`
- `file_stat`

### Disk Layout Changes
Modifications to the disk layout are made to accommodate file extensions. The use of additional fields in the `struct dinode` definition allows support for multiple extents.

- Add multiple extent support with a maximum of 30 extents.
- Keep the size of dinode struct a power of 2 to keep individual inodes from spanning multiple disk blocks.

### Journaling Implementation

The logging layer is introduced between the inode layer and block cache layer to implement journaling. Helper functions `begin_tx` and `commit_tx` are used to package transactions. The `log_write` wrapper function writes to the log region instead of the actual disk location. Successful transactions are identified by updating the log header on disk.

Journaling Steps:

- Write new blocks into the log instead of the target place.
- Track the target.
- Mark the log as "committed" once all blocks are in.
- Copy data from the log to their intended locations.
- Clear the commit flag.
- On system boot, check the log for consistency.

Log Structure:

- Initialize log using log_begin_tx().
- Use log_write() for block writes into the log.
- log_commit_tx() to complete a transaction and write out the commit block.
- log_apply() for log playback on system reboot.

### fs.c

We will need `locki` and `unlocki` to ensure synchronization when accessing the inode layer.

### bio.c

We will need `bread`, `bwrite`, and `brelse` for buffered I/O.

## Risk Analysis

### Unanswered Questions

- How to allocate the log region and how to implement log operations?

### Staging of Work

- First, we will implement the functions to enable writes and deletes.
- Then, we will implement synchronization to handle multiple processes.
- Finally, we will implement journaling to ensure crash safety.

### Time Estimation

Best / Average / Worst case scenario
- Functions
    - `write_file` (2/4/6 hours)
    - `append_file` (2/4/6 hours)
    - `create_file` (2/4/6 hours)
    - `sys_unlink` (2/4/6 hours)
- Synchronization
  - Design (2 hours)
  - Implementation (2/4/6 hours)
- Journaling
  - Design (4 hours)
  - Implementation (4/6/8 hours)
- Edge cases and Error handling (2 hours)

