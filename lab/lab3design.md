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

### Copy-on-write fork:
The Copy-on-write fork enhancement is aimed at optimizing the fork operation by introducing a copy-on-write mechanism. It enables the parent and child processes to share the same physical memory initially, reducing memory duplication and enhancing the overall efficiency of the fork process.

We need to add a bookkeeping field `cow_page` to `vpage_info` to indicate whether a page is copy-on-write. We also need to add a reference count to `core_map_entry` to keep track of the number of processes that are sharing the same physical memory.

- `fork`:
    - Allocate identical virtual space for the child process by point to corresponding physical space.
    - Set all pages in both the child and parent virtual spaces to be read-only initially. This ensures that any attempt to write to these pages triggers a page fault.
    - On a write attempt in child or parent, a page fault occurs.
    - Kernel allocates a new physical address for the virtual page using `vspaceaddmap`.
    - If the reference count for a physical page is 1, mark it as writable to enable modifications without additional page faults.

- `vspacecowcopy`:
    - Iterate through each region in the child vspace and copy the `vpage_info` pages from the corresponding parent region.
    - Mark the page table entries as read-only for both the child and parent.
    - Set the flag `vpi->cow_page` to true to indicate that the page is copy-on-write.
    - Update the reference count for the physical page.
    - Call `vspaceupdate` to ensure the child vspace is properly updated.

### Page Fault Handling
We need to distinguish between various page fault cases in `trap.c`:
#### Copy-on-Write Fork:

- Check if the faulted page is flagged as copy-on-write (`vpi->cow_page == true`).
- Verify if the page is currently shared among multiple processes (`entry->ref_count > 1`) and is marked as unwritable (`vpi->writable == 0`).
- If so, allocate a new physical page, copy data from the existing copy-on-write page, update reference counts, and set the page as writable for the faulting process.

#### Non-writable page with only 1 reference:

- Check if the faulted page is marked as copy-on-write and is the only reference to an unwritable page (`entry->ref_count == 1 && vpi->writable == 0`).
- If true, mark the page as writable for the faulting process.

#### Stack Growth on Demand:

- Detect if the faulted address is within a predefined range indicating proximity to the stack boundary.
- If true, attempt to grow the user stack using the `growuserstack` function.


### System Calls
#### sys_sbrk
The main goals of the `sys_*` functions is to do argument parsing and then calling the associated functions.

#### kalloc.c
- Update `core_map_entry` to include a refcount to store the number of processes that are sharing the same physical memory.
- Use `kmem.lock` to protect `core_map_entry` when updating the structure.

#### vspace.c
- Use `vspaceaddmap` to add mappings between virtual and physical addresses during page fault handling. 

#### vspace.h
- Add a new field `cow_page` to `vpage_info` to indicate whether a page is copy-on-write.


#### Helper Macros
- `P2V` physical address to virtual address
- `V2P` virtual address to physical address
- `PGNUM` physical address to page number
- `va2vpage_info` virtual address to `vpi_info`


## Risk Analysis

### Unanswered Questions
- Not sure if it is correct how we handle page faults depending on the type of page fault. We don't really use the bits in `tf->err`? In which case should it be handled by the kernel / growing stack / copy-on-write fork?


### Staging of Work
- Begin by implementing `sys_sbrk` to handle heap expansion.
- Modify `trap` to handle page faults and implement on-demand stack growth.
- Implement `vspacecowcopy` to copy the contents of the parent page to the child page similar to `vspacecopy`.
- Change `fork` to handle copy-on-write fork.



### Time Estimation
Best / Average / Worst case scenario
- Functions:
    - `sbrk`: (2/4/6 hours)
    - `trap`: (2/4/6 hours)
    - `cow_fork`: (6/8/10 hours)
