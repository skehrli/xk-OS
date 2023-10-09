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

int sys_dup(void) {
  // LAB1
  return -1;
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
  // you have to change the code in this function.
  // Currently it supports printing one character to the screen.

  int n;
  char *p;

  if (argint(2, &n) < 0 || argptr(1, &p, n) < 0)
    return -1;
  uartputc((int)(*p));
  return 1;

  /*
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
  */
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

int sys_fstat(void) {
  // LAB1
  return -1;
}

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
  if (namei(path) == NULL) {
    return -1;
  }

  // file system is read only, any write flags for non console files are invalid
  if (strncmp(path, "console", strlen(path)) != 0 && access_mode != O_RDONLY) {
    return -1;
  }

  // O_CREATE is not permitted (for now)
  if (access_mode == O_CREATE) {
    return -1;
  }

  // file_open returns an error if 
  // already at max open files or
  // there is no available file descriptor
  return file_open(path, access_mode);
}

int sys_exec(void) {
  // LAB2
  return -1;
}

int sys_pipe(void) {
  // LAB2
  return -1;
}

int sys_unlink(void) {
  // LAB 4
  return -1;
}
