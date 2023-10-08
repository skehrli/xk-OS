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

int sys_read(void) {
  // LAB1
  return -1;
}

int sys_write(void) {
  // you have to change the code in this function.
  // Currently it supports printing one character to the screen.

  int n;
  char *p;

  if (argint(2, &n) < 0 || argptr(1, &p, n) < 0)
    return -1;
  uartputc((int)(*p));
  return 1;
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
