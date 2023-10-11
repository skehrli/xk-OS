## Part 1
### Question 1
- a) 24: cprintf("\ncpu%d: starting xk\n\n", cpunum());
- b) 23: e820_print();
### Question 2
We can get the memory address of the main function with the command:
x/x main
Which prints out the byte of memory at the symbol main.
The output is: 
0xffffffff801026bc <main>:      0xe5894855
Meaning that the main function is at address 0xffffffff801026bc.

GDB can work both with physical and virtual addresses in the qemu environment. The command:
maintenance packet qqemu.PhyMemMode
Outputs either 0 (virtual addressing) or 1 (physical addressing). The default value is 0, so the answer is virtual addressing by default. This information can be found on the qemu gitlab page.

## Part 2
### Question 3
Why does xk map kernel and user-application into the same address space? (Why does the kernel not have a separate address space?)
- So that the system call can
  access the process' memory.
  If it had it's own address
  space, it wouldn't be able
  to do so.
- This means that upon a
  switch to kernel mode, the
  address space need not
  be switched, which means
  faster
  usermode-kernelmode
  switches.

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

## Part 4
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
