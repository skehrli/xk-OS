# Lab 2: Multiprocessing
## Part 1 Design Doc Due: 10/20/23 at 11:59pm. No late days.
## Part 1 Code Due: 10/27/23 at 11:59pm.
## Part 2 Design Doc Due: 10/27/23 at 11:59pm. No late days.
## Part 2 Code Due: 11/3/23 at 11:59pm.

## Introduction
In lab1, there is only one process running on our OS, the init process. 
In this lab, you will implement new system calls to enable multiprocessing (part 1) and inter-process communication (part 2).

Before you start implementing lab2, you should revisit your lab1 system calls 
to make sure that it handles concurrent access.
(Even without multiple processors, the timer may expire during a system call,
causing a context switch to a new process.) 

For part 1 of this lab, you will implement the UNIX `fork`, `wait`, and `exit` to enable multiprocessing.
Fork creates a copy of the current process (parent) and returns in both the parent process and the child process with different values.
Wait blocks a process until one of its child processes finishes executing.
Exit should not ever return, the exiting process should cleanly free its resources (eg. open files),
and return kernel allocated pages back to the kernel heap.   

For part 2 of this lab, you will you will implement basic interprocess communication through pipes, 
and further enhance multiprocessing with `exec`. Pipe allows for transfer of data
from one process to another. UNIX `exec` loads a new program onto an existing process. 
`pipe`, `fork`, and `exec` are often used together for a parent process to create a child process 
running different code and communicate via the pipe.

From lab2 on, we will be asking you to write a small design document.  Follow
the guidelines in [how to write a design document](designdoc.md). When finished
with your design document, submit it on Gradescope as a pdf. 

Just in case we have problem with Gradescope, please also store a copy of your
design doc pdf in the `lab/` folder and tag your repo with:

```
git tag lab2_design
git push origin main --tags
```

## Configuration
To get lab2, run the command
```
git pull upstream main
```
and merge.

If your repo doesn't have the class repo as upstream, make sure to add it via
```
git remote add upstream git@gitlab.cs.washington.edu:xk-public/23au.git
```
and then pull and merge.

After the merge, run `make qemu` to make sure that your code still passes lab1 tests before you continue.

## Lab 2 testing
**After** you add synchronization to files and run `make qemu` to check lab1 tests,
you can replace `lab1test` with `lab2test` in `kernel/Makefrag` to test lab2 code.
```
$(O)/initcode : kernel/initcode.S user/lab1test.c $(ULIB)
	$(CC) -g -nostdinc -I inc -c kernel/initcode.S -o $(O)/initcode.o
	$(CC) -g -ffreestanding -MD -MP -mno-sse -I inc -c user/lab1test.c -o $(O)/lab1test.o
	$(LD) $(LDFLAGS) -N -e start -Ttext 0x10000 -o $(O)/initcode.out $(O)/initcode.o $(O)/lab1test.o $(ULIB)
	$(OBJCOPY) -S -O binary $(O)/initcode.out $(O)/initcode
	$(OBJDUMP) -S $(O)/initcode.out > $(O)/initcode.asm
```
This will create a new `initcode` binary which has lab2test in it, making sure the init process will run lab2test.c.

The lab2test program allows you to run `part1`, `part2`, `all`, and individual tests (function name).
It also understands the `exit` command when you are done testing.
You will need `fork` working in order for the test program to run. 
If your fork can successfully return to userspace with the correct return value, you should see a lab2 shell prompt `(lab2) >`.

## Part 1: Enabling Multiprocessing

### Add synchronization to files
Once multiple processes are concurrently running on xk,
xk must ensure any shared data structures are properly protected.
xk provides two types of locks for mutual exclusion, spinlock (`kernel/spinlock.c`) and sleeplock (`kernel/sleeplock.c`).

When acquiring a spinlock, the process spins (loops) on the cpu while waiting for the lock to be free.
This wastes CPU cycles but is ok if the critical section protected by the lock is small and the wait is short.
An example usage of spinlock in xk is the `ptable` (`kernel/proc.c`). 
`ptable` is used for allocation and scheduling of processes. The critical section is small, all operations are in memory,
and it's used by the scheduler which often runs inside an interrupt context and cannot block.

When acquiring a sleeplock, the process sleeps (gives up the cpu) while waiting for the lock to be free.
This avoids wasting CPU cycles while waiting, but can add additional context switching overhead if the wait is short.
An example usage of sleeplock in xk is the `struct inode` (`kernel/fs.c`). 
Operations on an inode is protected by the inode lock via `locki`. Since inode operations can result in disk accesses (slow),
it makes sense for the processing waiting for the inode lock to sleep.

In lab1, you introduced a shared data structure (file_info table) into the kernel.
Now you have to make sure the shared structure is protected in the face of concurrent accesses. 
Think carefully about what type of lock to use and the granularity of your lock(s).

Please document and justify your locking scheme in the design document. 
Specify what types of locks you use and what scenarios they protect against.

### Implement the fork, wait, and exit system calls
These system calls work together to support multiprocessing.
You can start by implementing fork, but one of the first tests will
have the new process call process exit.

#### fork: creating a new process
When fork is called, a new process is created as a copy of the calling process.
This might seem a bit silly (why do we want to have two copies of the same process?), 
but since the child process is a copy, it inherits resources (eg. open files) from its parent.
This allows the parent process to configure the child's environments and makes writing a shell program much easier.
Inheriting a set of open files means that calling read in the parent should 
advance the offset in both the parent and the child.

The system call fork returns twice, once in the parent, with the return value of 
the process ID (pid) of the child, and once in the child, with the return value of 0.

Once a new process is created and set as RUNNABLE, xk will run it concurrently via the
process scheduler. A hardware device generates a timer interrupt on
fixed intervals. If another process is RUNNABLE, the scheduler will switch
to it, essentially causing the current process to yield the CPU.

You need to implement `sys_fork` in `kernel/sysproc.c` and `fork` in `kernel/proc.c`.
`vspaceinit` and `vspacecopy` might be useful.

```c
/*
 * Creates a new process as a copy of the current process, with
 * the same open file descriptors.
 *
 * On success, parent returns with the process ID of the child,
 * child should be runnable and returns just like the parent, but with a value of 0.
 * Returns -1 on error.
 *
 * Errors:
 * kernel lacks space to create new process
 */
int
sys_fork(void);
```

#### Question #1
What is the functionality for each of the following function: `scheduler`, `sched` and `swtch` (in `kernel/proc.c`)
Please also describe the relationship between them.

#### Question #2
How does `fork` return both in the parent process and the child process?
Describe the execution flow when the child process return to user-level application after `fork` is called.
Hint: take a look at the end of function `allocproc`.

#### wait and exit: synchronization between parent and child process
One of the first things our test cases will do is to have the forked child
call process `exit`. You need to implement exit to cleanly return the
allocated memory back to the kernel heap.  A complication is that the
process calling exit cannot free all of its memory, as it is running on
a stack in a process context, and e.g., the timer interrupt code will
assume that every interruptable process can be time sliced.

The textbook describes how to address this: when a process exits,
mark the process to be a `ZOMBIE`. This means that someone else (e.g.,
the process parent or the next process to run) is responsible for cleaning
up its memory.

The `wait` system call interacts with fork and exit: it is called by
the parent process to wait for a child. Note that a parent
can create multiple child processes. Wait does not have any argument.
Kernel finds one exited child and return the child's pid.
Further note that you need to be careful with synchronization here: the
parent may call wait before any child calls exit.  In that case, the wait
should stall (e.g., use a condition variable) until one child exits.

Note that the parent need not call wait; it can exit without waiting
for the child. The child's data structures must still be reclaimed
in this case when the child does eventually exit.

There are various ways that you can implement all of this, so you should
think through the various cases and design an implementation before you start to code.
If you choose to let the parent clean up its exited children,
you will need some way to reclaim the children's
data when the parent exits first. (Note that in xk as in UNIX,
the init process never exits.)

You need to implement `sys_wait` and `sys_exit` in `kernel/sysproc.c`,
and `wait` and `exit` in `kernel/proc.c`.

```c
/*
 * Halts program and reclaims resources consumed by program.
 * Does not return.
 */
void
sys_exit(void);

/*
 * No arguments. Suspend execution until a child terminates (calls exit
 * or is killed). Returns process ID of that child process; hangs if
 * no child process has terminated (until one does); returns -1 on error.
 * A child is returned from wait only once.
 *
 * Error conditions:
 * The calling process did not create any child processes, or all
 * child processes have been returned in previous calls to wait.
 */
int
sys_wait(void);
```

#### Question #3

How does the `kill` system call work in xk? What is its relationship with `exit`?

## Part 2: Inter-process Communication

### Pipes
A pipe is a sequential communication 
channel between two endpoints, supporting writes on one end, and 
reads on the other. Reads and writes are asynchronous and buffered, 
up to some internally defined size. A read will block until there are 
bytes to read (it may return a partial read); a write will block if there is
no more room in the internal buffer. Pipes are a simple way to support interprocess 
communication, especially between processes started by the shell, with the system
calls you just implemented. 

The `pipe` system call creates a pipe (a holding area for written
data) and opens two file descriptors, one for reading and one for writing.
A write to a pipe with no open read descriptors will return an error.
A read from a pipe with no open write descriptors will return any
remaining buffered data, and then 0 to indicate EOF.

You should be able to write any number of bytes to a pipe. Concurrent operations can happen 
in any order, but each operation completes as an atomic unit. This means that 
if writer A tries to write `aaa` and writer B tries to write `bbb` concurrently,
a reader can read `aaabbb` or `bbbaaa`, but not interleaved like `abbbaab`.

> Note: xk differs from POSIX semantics by disallowing partial writes. This
> change was made for pedagogical reasons to ensure students learns how to use
> monitors.

You need to implement `sys_pipe` in `kernel/sysfile.c`. After you implement pipe, 
revisit your lab 1 solution to make sure that `read` `write` `close` `dup` works with pipe.

#### Implementation tips
You can reference the bounded buffer in Chapter 5 of OSPP and the lecture notes.
We don't have a size limit for your pipe's buffer, but you may find it convenient
to allocate a 4KB page (via `kernel/kalloc.c:kalloc`) for both metadata and the data of your pipe.
The actual buffer may be smaller than a page as you will need room for metadata about the pipe.

Since pipes support concurrency, your pipe implementation will
need to use spinlocks and condition variables. Wth `dup` you may also have multiple readers and writers on the same pipe.
In xk, `wakeup1` is the equivalent of `cv::broadcast` (there is no signal -- all waiters are woken up).
`sleep` is the equivalent of `cv::wait`; it puts the process to sleep and releases the spinlock, and re-acquires the lock on wakeup.  

```c
/*
 * arg0: int * [2] [pointer to an array of two file descriptors]
 *
 * Creates a pipe and two open file descriptors. The file descriptors
 * are written to the array at arg0, with arg0[0] the read end of the 
 * pipe and arg0[1] as the write end of the pipe.
 *
 * return 0 on success; returns -1 on error
 *
 * Errors:
 * Some address within [arg0, arg0+2*sizeof(int)] is invalid
 * kernel does not have space to create pipe
 * kernel does not have two available file descriptors
 */
int
sys_pipe(void);
```

### Execute user program

User programs are in the file system using “Executable and Linkable Format”
(i.e., ELF). Full information about this format is available in the
[ELF specification](https://courses.cs.washington.edu/courses/cse451/16au/readings/elf.pdf)
but you will not need to delve very deeply into the details of this format in this
class. We have provided functions (i.e., `vspaceloadcode` in `kernel/vspace.c`)
to read the program and load it into the passed in address space. We also have 
provide functions (i.e., `vspaceinitstack`) to setup and initialize the user stack.

You need to setup all the required kernel data structures for the new program.
This should be slightly similar to `fork`, but the key differences are explained
below.

#### Switch to a new address space
Unlike `fork`, `exec` does not create a new process but overwrites the existing process's virtual address space 
with a new code (`vspaceloadcode`) and stack section (see the next section). 
You need to first create a new vspace (`vspaceinit`), set up its code and stack region, 
set the new vspace as the current process's `vspace`, and activate it via `vspaceinstall`. 
Make sure you also free (`vspacefree`) the old vspace to avoid memory leaks.
Think carefully about when you free the old vspace. The ordering of these steps is crucial. 

#### Question #4
`vspace` holds the metadata of a process's virtual address space and the pointer to the process's machine dependent page table.
What is being freed in `vspacefree`? What happens if you try to free the current process's vspace? 
Try doing `vspacefree(&myproc()->vspace)` at the beginning of `exec` and then run exec tests. Explain the output you observe.

#### Set up exec arguments
One thing special about `exec` compared to `fork` is the need to pass in the
arguments of the user program. In order to realize this functionality, you need to
first pull the arguments from the `exec`-ing program. With these arguments, you need to
carefully construct the user process stack and register state after loading the program
to give the `main` function in the loaded program its arguments. 
To initialize the user stack you may use `vspaceinitstack`. 
In xk, we cap the user stack to be 2GB (`SZ_2G` in `inc/cdefs.h`), see details in [memory.md](memory.md). 
The arguments that xk pulled from the exec-ing program and the arguments that xk is giving to the new
user-level program are both in user memory. However, simply copying the pointers is
not sufficient because the page tables of the two user-level programs are different.
You must create a deep copy of the argument from the old address space to the new address space
(see `kernel/vspace:vspacewritetova`).

`exec` has two arguments, a path to the executable ELF file and a pointer `argv` to
an array of strings (recall that a "string" in C is a `char *`). For example, let's say
we run:
```
$ cat a.txt b.txt
```
In the `exec` syscall, the first argument is the path to `cat` program. The second argument
is an array of `char *` where the first points to string `cat`, the second points to
string `a.txt`, the third points to string `b.txt` and the fourth element is `\0`
indicating the end of the array.

      argv
        |
        |     0   1   2   3
        |   +---+---+---+----+
        +-> | * | * | * | \0 |
            +---+---+---+----+
              |   |   |
              |   |   |
              |   |   +-> "b.txt"
              |   |
              |   +-> "a.txt"
              |
              +-> "cat"

When `exec` setups the user stack, we need to be careful. Note that every user program in
xk has the same definition of main (except the testing scripts, because we didn't load
testing scripts via `exec` yet).

``` C
int
main(int argc, char *argv[])
```

This means the first argument, `argc`, is the length of `argv`, where `argv` is a pointer
to a list of strings. In the previous example, this means you have to copy `cat`, `a.txt`
and `b.txt` to the user stack. Create an array on the user stack whose 0th index points
to `cat`, 1st index points to `a.txt`, 2nd index points to `b.txt` and 
3rd index element is the 0 pointer `\0`. 

Once you've set up the stack, remember to Set the `%rdi` register (first argument in x86\_64 
calling convention) to be 3 (length of argv) and the `%rsi` register (second argument in
x86\_64 calling convention) to the `argv` array you created on the stack. 
You also need to update other fields in the trapframe to ensure that the process returns to
user mode with its new entry point and uses the new stack. 
It might be helpful to take a look at how `userinit` sets up the trapframe.

You need to implement `sys_exec` in `kernel/sysfile.c` and `exec` in `kernel/exec.c`.
Note that the `exec_ls` and `exec_echo` test require `pipe` to be implemented.
Hint: You shouldn't call `vspacecopy` in `exec`.

```c
/*
 * arg0: char * [path to the executable file]
 * arg1: char * [] [array of strings for arguments]
 *
 * Given a pathname for an executable file, sys_exec() runs that file
 * in the context of the current process (e.g., with the same open file 
 * descriptors). arg1 is an array of strings; arg1[0] is the name of the 
 * file; arg1[1] is the first argument; arg1[n] is `\0' signalling the
 * end of the arguments.
 *
 * Does not return on success; returns -1 on error
 *
 * Errors:
 * arg0 points to an invalid or unmapped address
 * there is an invalid address before the end of the arg0 string
 * arg0 is not a valid executable file, or it cannot be opened
 * the kernel lacks space to execute the program
 * arg1 points to an invalid or unmapped address
 * there is an invalid address between arg1 and the first n st arg1[n] == `\0'
 * for any i < n, there is an invalid address between arg1[i] and the first `\0'
 */
int
sys_exec(void);
```

#### exec stack layout
Call `vspacedumpstack` with the newly created vspace to observe the stack contents. \
This is NOT the only possible stack format. \
Here is a solution code's stack layout for `ls`:
```
exectest: testing ls
dumping stack: base=80000000 size=4096
virtual address: 7ffffff8 data: 736c
virtual address: 7ffffff0 data: 0
virtual address: 7fffffe8 data: 7ffffff8
virtual address: 7fffffe0 data: 0
```
And for `echo` `echotest` `ok`:
```
exectest: test argument
dumping stack: base=80000000 size=4096
virtual address: 7ffffff8 data: 6b6f
virtual address: 7ffffff0 data: 0
virtual address: 7fffffe8 data: 747365746f686365
virtual address: 7fffffe0 data: 6f686365
virtual address: 7fffffd8 data: 0
virtual address: 7fffffd0 data: 7ffffff8
virtual address: 7fffffc8 data: 7fffffe8
virtual address: 7fffffc0 data: 7fffffe0
virtual address: 7fffffb8 data: 0
```

#### Question #5
For each member of the project team, how many hours did you
spend on this lab?

#### Question #6
What did you like or dislike about this lab? Is there anything you wish you knew earlier?

## Testing and hand-in

### Part 1 expected output
After part 1 is implemented, running `part1` should have the following output.
```
(lab2) > part1
fork_wait_exit_basic -> passed 
fork_wait_exit_cleanup -> passed 
fork_wait_exit_multiple -> passed 
fork_wait_exit_50 -> passed 
fork_wait_exit_tree -> passed 
fork_fd_test -> passed 
passed lab2 part1 tests!
```
We will run both individual tests and all part1 tests.

### Part 2 expected output
After part 2 is implemented, running `part2` should be close to the following: 
```
(lab2) > part2
pipe_test -> passed 
pipe_closed_ends -> passed 
exec_bad_args -> passed 
exec_ls -> 
------ ls output -------
.              1 1 400
..             1 1 400
console        3 2 0
lab1test       2 3 42544
rm             2 4 22248
cat            2 5 22632
zombie         2 6 22096
echo           2 7 22216
lab3test       2 8 33896
kill           2 9 22296
lab2test       2 10 45832
lab3init       2 11 22384
init           2 12 22528
stressfs       2 13 22672
sysinfo        2 14 22432
ln             2 15 22216
wc             2 16 22816
lab4test_a     2 17 43824
lab4test_c     2 18 27856
lab4test_b     2 19 34808
ls             2 20 23344
grep           2 21 23400
sh             2 22 31456
small.txt      2 23 26
l2_share.txt   2 24 20
-------------------------
passed 
exec_echo -> passed 
kill_test -> passed 
passed lab2 part2 tests!
```
We will run both individual tests and all part2 tests.

Your ls output may have different values for the last column but should have the same set of entries (unless you are writing more user applications yourself).

Please submit your answers to the questions listed above to Gradescope.
To submit the code, download your repo as a zip file and submit to Gradescope.

Just in case we have problem with Gradescope, please store a copy of your writeup in a file `lab2.txt` in the top-level xk directory and create a `end_lab2` git tag.
