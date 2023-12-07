//
// File-system system calls.
// Mostly argument checking, since we don't trust
// user code, and calls into file.c and fs.c.
//

#include <cdefs.h>
#include <defs.h>
#include <fcntl.h>
#include <file.h>
#include <fs.h>
#include <mmu.h>
#include <param.h>
#include <proc.h>
#include <sleeplock.h>
#include <spinlock.h>
#include <stat.h>

/*
 * arg0: char * [path to the file]
 * arg1: int [mode for opening the file (see inc/fcntl.h)]
 *
 * Given a pathname for a file, sys_open() returns a file descriptor, a small,
 * nonnegative integer for use in subsequent system calls. The file descriptor
 * returned by a successful call will be the lowest-numbered file descriptor
 * not currently open for the process.
 *
 * Each open file maintains a current position, initially zero.
 *
 * returns -1 on error
 *
 * Errors:
 * arg0 points to an invalid or unmapped address 
 * there is an invalid address before the end of the string 
 * the file does not exist
 * already at max open files
 * there is no available file descriptor 
 * since the file system is read only, any write flags for non console files are invalid
 * O_CREATE is not permitted (for now)
 *
 */
int sys_open(void) {
  // LAB1
  char *path;
  int access_mode;

  // fetch the syscall arguments
  if (argstr(0, &path) < 0 || argint(1, &access_mode) < 0) {
    // arguments fetching fails
    return -1;
  }

  // arg0 points to an invalid or unmapped address or,
  // there is an invalid address before the end of the string or,
  // the file does not exist
  // (LAB4) support file creation
  /*
  if (namei(path) == NULL && (access_mode & 0xF00) != O_CREATE) {
    return -1;
  }
  */

  // (LAB1) file system is read only, any write flags for non console files are invalid
  /* (LAB4) make file system writeable
  if (strncmp(path, "console", strlen(path)) != 0 && access_mode != O_RDONLY) {
    return -1;
  }
  */

  // (LAB1) O_CREATE is not permitted
  /* (LAB4) O_CREATE is permitted
  if (access_mode == O_CREATE) {
    return -1;
  }
  */

  // file_open returns an error if 
  // already at max open files or
  // there is no available file descriptor
  return file_open(access_mode, path);
}


/*
 * arg0: int [file descriptor]
 *
 * Duplicate the file descriptor arg0, must use the smallest unused file descriptor.
 * Return a new file descriptor of the duplicated file, -1 otherwise
 *
 * dup is generally used by the shell to configure stdin/stdout between
 * two programs connected by a pipe (lab 2).  For example, "ls | more"
 * creates two programs, ls and more, where the stdout of ls is sent
 * as the stdin of more.  The parent (shell) first creates a pipe 
 * creating two new open file descriptors, and then create the two children. 
 * Child processes inherit file descriptors, so each child process can 
 * use dup to install each end of the pipe as stdin or stdout, and then
 * close the pipe.
 *
 * Error conditions:
 * arg0 is not an open file descriptor
 * there is no available file descriptor
 */
int sys_dup(void) {
  // LAB1
  int fd;
  if (argfd(0, &fd) < 0) {  
    // arg0 is not an open file descriptor or there is no available file descriptor
    return -1;
  }
  return file_dup(fd);
}

/*
 * arg0: int [file descriptor]
 *
 * Close the given file descriptor
 * Return 0 on successful close, -1 otherwise
 *
 * Error conditions:
 * arg0 is not an open file descriptor
 */
int sys_close(void) {
  // LAB1
  int fd;

  // arg0 is not an open file descriptor for the current process
  if (argfd(0, &fd) < 0) {
    return -1;
  }

  return file_close(fd);
}


/*
 * arg0: int [file descriptor]
 * arg1: char * [buffer to write read bytes to]
 * arg2: int [number of bytes to read]
 *
 * Read up to arg2 bytes from the current position of the corresponding file of the 
 * arg0 file descriptor, place those bytes into the arg1 buffer.
 * The current position of the open file is then updated with the number of bytes read.
 *
 * Return the number of bytes read, or -1 if there was an error.
 *
 * Fewer than arg2 bytes might be read due to these conditions:
 * If the current position + arg2 is beyond the end of the file.
 * If this is a pipe or console device and fewer than arg2 bytes are available 
 * If this is a pipe and the other end of the pipe has been closed.
 *
 * Error conditions:
 * arg0 is not a file descriptor open for read 
 * some address between [arg1, arg1+arg2) is invalid
 * arg2 is not positive
 */
int sys_read(void) {
  // LAB1
  int fd;
  int n;
  char *buf;

  if (argfd(0, &fd) < 0 ||    // arg0 is not a file descriptor
      argint(2, &n) < 0 ||    // arg2 is not positive
      argptr(1, &buf, n) < 0  // some address between [arg1, arg1+arg2) is invalid
      ) {
    return -1;
  }  

  return file_read(fd, buf, n);

}

/*
 * arg0: int [file descriptor]
 * arg1: char * [buffer of bytes to write to the given fd]
 * arg2: int [number of bytes to write]
 *
 * Write up to arg2 bytes from arg1 to the current position of the corresponding file of
 * the file descriptor. The current position of the file is updated by the number of bytes written.
 *
 * Return the number of bytes written, or -1 if there was an error.
 *
 * If the full write cannot be completed, write as many as possible 
 * before returning with that number of bytes.
 *
 * If writing to a pipe and the other end of the pipe is closed,
 * return -1.
 *
 * Error conditions:
 * arg0 is not a file descriptor open for write
 * some address between [arg1,arg1+arg2-1] is invalid
 * arg2 is not positive
 *
 * note that for lab1, the file system does not support writing past 
 * the end of the file. Normally this would extend the size of the file
 * allowing the write to complete, to the maximum extent possible 
 * provided there is space on the disk.
 */
int sys_write(void) {
  // LAB1
  int fd;
  int n;
  char *buf;

  if (argfd(0, &fd) < 0 ||    // arg0 is not a file descriptor
      argint(2, &n) < 0 ||    // arg2 is not positive
      argptr(1, &buf, n) < 0  // some address between [arg1, arg1+arg2) is invalid
      ) {
    return -1;
  }
  
  return file_write(fd, buf, n);
}

/*
 * arg0: int [file descriptor]
 * arg1: struct stat *
 *
 * Populate the struct stat pointer passed in to the function
 *
 * Return 0 on success, -1 otherwise
 *
 * Error conditions: 
 * if arg0 is not a valid file descriptor
 * if any address within the range [arg1, arg1+sizeof(struct stat)] is invalid
 */
int sys_fstat(void) {
  // LAB1
  int fd;
  struct stat *stat_ptr;
  if (argfd(0, &fd) < 0 ||    // arg0 is not a file descriptor
      argptr(1, (char **)(&stat_ptr), sizeof(struct stat)) < 0  // some address within the range [arg1, arg1+sizeof(struct stat)] is invalid
  ) {
    return -1;
  }

  return file_stat(fd, stat_ptr);
}

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
int sys_exec(void) {
  // LAB2
  char *path;
  char *argv[MAXARG];

  if (argstr(0, &path) < 0 ||  // arg0 points to an invalid or unmapped address
      argptr(1, (char**)&argv, sizeof(char**)) < 0  // arg1 points to an invalid or unmapped address
  ) {
    // invalid arguments
    return -1;
  }

  // Get address of arguments array
  int64_t argv_addr;
  if(argint64(1, &argv_addr) < 0) {
    return -1;
  }

  // Loop through the arguments
  for(int i = 0; i < MAXARG; i++) {
    // Get address of each argument
    int addr;
    if(fetchint((uint64_t)argv_addr + sizeof(char*) * i, &addr) < 0) {
      // check if the address is valid
      return -1;
    }

    // Check if the address is 0 because argv is NULL terminated
    if (addr == 0) {
      argv[i] = '\0';
      break;
    }

    // Get string pointed at address
    if(fetchstr((uint64_t)addr, &argv[i]) < 0) {
      return -1;
    }

    // End of argv
    if(argv[i] == '\0') {
      break;
    }
  }

  return exec(path, argv);
}

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
int sys_pipe(void) {
  // LAB2
  int *fd_arr;
  if (argptr(0, (char **)&fd_arr, 2*sizeof(int)) < 0) {
    // invalid arguments
    return -1;
  }
  return pipe(fd_arr);
}

/*
 * arg0: char * [path to the file]
 * 
 * Given a pathname for a file, if no process has an open reference to the
 * file, sys_unlink() removes the file from the file system.
 *
 * On success, returns 0. On error, returns -1.
 *
 * Errors:
 * arg0 points to an invalid or unmapped address
 * there is an invalid address before the end of the string
 * the file does not exist
 * the path represents a directory or device
 * the file currently has an open reference
 */
int sys_unlink(void) {
  // LAB 4
  char * path;

  if (argstr(0, &path) < 0) {
    // invalid arguments
    return -1;
  }

  return file_unlink(path);
}
