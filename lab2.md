### Question 1
Scheduler() is the method that a CPU calls initially (and which never returns). The scheduler() method chooses the first runnable process and runs it by instaling its address space and swtch()ing from the scheduler context to the process context. Eventually, the process swtch()es back to the scheduler() and it starts over, selecting the first runnable process and so on.
Swtch() is then the assembly code that executes the context switch from the one context to the other.
When a process yields the CPU, it doesnt directly swtch() to the scheduler() process/thread, but it calls sched() instead, which then swtch() es to the scheduler(). The call to swtch() needs to be wrapped into the sched() method because the interrupt enable flag of the kernel process needs to be restored after the call to swtch(), which is what sched() does.

### Question 2
For the process who calls fork(), the function returns normally with a return value.
The return of the forked child process is constructed:
First, allocproc() allocates all resources the process needs, like a kernel stack, trap frame and context, such that the process can start running in kernel space to finish the setup.
Then, the entire address space of the parent and the trapframe of the parent is copied. This way, the new process will behave exactly like its parent when returning to userspace (same instruction pointer, registers and everything). The only difference is that fork() also overrides the rax register (where the program expects the return value by x86 convention) with 0. The fork() method also initializes the pcb of the child process and sets it to runnable, such that it can now be scheduled. Now the fork() returns to the parent process with the pid of the child.

### Question 3
The kill system call sets the killed flag in the pcb of the targeted process and wakes it up if it is sleeping. When the killed process is scheduled the next time, the trap kernel method checks the killed flag and calls exit() if it is set.

### Question 4
`vspacefree` frees the memory pages, virtual address space metadata, and the page table associated with the given `vspace`, cleaning up the resources used by the process's virtual address space and page table.

If we free the current process's vspace at the beginning of `exec`, the program restarts from the beginning, because the current running process' memory pages and page table are freed. The program will then be loaded into a new vspace and is initialized again.

### Question 5
- Sascha ~12 hours
- Daniel ~16 hours

### Question 6
Daniel: I found the lab very interesting. The diagram and samples given were very helpful when testing. Maybe a more in depth explanation of how to setup the user stack would have been useful.

