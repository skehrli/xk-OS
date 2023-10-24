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
- kalloc is used to allocate
  pages for kernel
  datastructures and when
  processes need more memory,
  while malloc is used to
  allocate memory in a
  process' heap (virtually).
  Physically, these are
  already allocated (physical)
  pages.
- printf prints text to the
  console in user-level
  programs, while cprintf is
  used for printing debug
  information inside the
  kernel.

### Question 5
The first c code executed is in the kernel/trap.c file, the trap() function. The first line of c code in this function is the declaration uint64_t addr;. If we don't count declarations, it is the if-statement:
if (tf->trapno == TRAP_SYSCALL) {

### Question 6
We can find the declaration of the trap_frame struct in inc/trap.h:
struct trap_frame {
▏ uint64_t rax; // rax
▏ uint64_t rbx;
▏ uint64_t rcx;
▏ uint64_t rdx;
▏ uint64_t rbp;
▏ uint64_t rsi;
▏ uint64_t rdi;
▏ uint64_t r8;
▏ uint64_t r9;
▏ uint64_t r10;
▏ uint64_t r11;
▏ uint64_t r12;
▏ uint64_t r13;
▏ uint64_t r14;
▏ uint64_t r15;
▏ uint64_t trapno;
▏ /* error code, pushed by hardware or 0 by software
▏ uint64_t err;
▏ uint64_t rip;
▏ uint64_t cs;
▏ uint64_t rflags;
▏ /* ss:rsp is always pushed in long mode */
▏ uint64_t rsp;
▏ uint64_t ss;
} __packed;
Since it contains 22 64 bit integers, the trap frame has size 22*8=176 Byte.

### Question 7
The backtrace shows the following content:
(gdb) bt
#0  sys_sleep () at kernel/sysproc.c:68
#1  0xffffffff8010594c in syscall ()
    at kernel/syscall.c:165
#2  0xffffffff80107f88 in trap (tf=0xffffffff8015af50)
    at kernel/trap.c:39
#3  0xffffffff80101634 in alltraps ()
    at kernel/trapasm.S:20
#4  0x000000000000000d in ?? ()
#5  0x0000000000000000 in ?? ()

We can see that the reported kernel function are:
- sys_sleep() is the kernel implementation of the sleep system call
- sys_sleep() was called in the syscall() function in kernel/syscall.c
- syscall() was called in alltraps() in kernel/trapasm.S. This is the function that stores the trap frame on the kernel stack.

### Question 8
No, it is not safe. The specifics depend on the system call and its argument. A few examples:
- for system calls like fork or kill, the kernel needs to check whether the process has permissions for that
- if the system call includes memory addresses, the kernel needs to check whether the process is allowed to read. Else, the process might read data it is not supposed to read or override critical data
- when a buffer to read/write is passed, the kernel must check whether the indicated length does not surpass the size of the buffer. When this is not checked, it can be a buffer overflow attack and might override return addresses so that the kernel executes malicious code
- for filesystem related system calls, the kernel needs to validate whether the passed file descriptor corresponds to a file that is valid
For each of these cases, the kernel can simply validate the arguments and decide to return an error if the arguments are invalid.

### Question 9
The problem with returning the entire struct is that it contains a pointer to the global file table. If we return the struct, a user process can access the entire file table (since arrays are contiguous in memory) and hence access files of other processes, which is not allowed. There is not really a benefit of returning the struct anyways. The handle is all the process needs.

### Question 10
- Sascha ~14 hours
- Daniel ~10 hours

### Question 11
- Sascha: I really liked the lab. There maybe could be a section about debugging. For example, a problem we both faced was that implementing write() did NOT work. No matter what we tried. It turned out that it didn't work because we were supposed to implement dup() before that. There could be a (stronger) hint for the required order in which the syscalls need to be implemented. The documentation in the design document was good and didn't really leave things unspecified.
