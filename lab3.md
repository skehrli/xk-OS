### Question 1
`malloc` implementations can dynamically adjust memory allocation strategies and make a standard for the allocated memory size to minimize fragmentation and improve overall memory utilization. These strategies are often more efficient than a simple linear growth strategy, which is what `sbrk` provides.

The relationship between `malloc`/`free` and `sbrk` is that `malloc` and `free` are built on top system calls like `sbrk`. `malloc` manages the heap and calls `sbrk` when more memory is needed, while `free` returns memory to the heap.

### Question 2
The command `ls|wc` is a pipe that connects the output of `ls` to the input of `wc`. The shell program will setup a pipe by having two file descriptors, one for the read end and one for the write end. To do that the process will setup the two file descriptors and then call fork to create a child process and the parent will close the read end and the child will close the write end. The command `ls` lists the files in the current directory and passes it's output to `wc` which counts the number of lines, words, and bytes in the input.

### Question 3
After a page fault exception is resolved, user-level execution resumes at the instruction that initially caused the page fault. This is because the instruction couldn't be executed successfully due to the absence of the required page in memory. The page fault handler ensures the necessary page is brought into memory, allowing the user-level code to resume executing from the interrupted instruction.