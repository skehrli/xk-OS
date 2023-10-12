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
  if (argint(0, &fd) < 0) {
    return -1;
  }
  if (fd < 0 || fd >= NOFILE) {
    return -1;
  }
  return file_dup(fd);
}

int sys_read(void) {
  // LAB1
  int fd, nr_bytes;
  char *buf;
  if (argint(0, &fd) < 0) {
    return -1;
  }
  if ((fd < 0) || (fd >= NOFILE)) {
    return -1;
  }
  if (argint(2, &nr_bytes) < 0) {
    return -1;
  }
  if (argptr(1, &buf, nr_bytes) < 0) {
    return -1;
  }
  return file_read(fd, buf, nr_bytes);
}

int sys_write(void) {
  int fd, nr_bytes;
  char *buf;

  if (argint(0, &fd) < 0) {
    return -1;
  }
  if ((fd < 0) || (fd >= NOFILE)) {
    return -1;
  }
  if (argint(2, &nr_bytes) < 0 || argptr(1, &buf, nr_bytes) < 0)
    return -1;
  return file_write(fd, buf, nr_bytes);
}

int sys_close(void) {
  // LAB1
  int fd;
  if (argint(0, &fd) < 0) {
    return -1;
  }
  if ((fd < 0) || (fd >= NOFILE)) {
    return -1;
  }
  return file_close(fd);
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
  if (argint(0, &fd) < 0) {
    return -1;
  }
  if (argptr(1, (char **)(&stat_ptr), sizeof(struct stat)) == -1) {
    return -1;
  }
  return file_stat(fd, stat_ptr);
}

int sys_open(void) {
  // LAB1
  char *file_path;
  int file_mode;

  if (argstr(0, &file_path) < 0 || argint(1, &file_mode) < 0) {
    // invalid arguments
    return -1;
  }

  // check validity of the file_mode argument
  if (file_mode == O_CREATE) {
    // not allowed currently
    return -1;
  }
  if ((file_mode != O_RDONLY) && (file_mode != O_WRONLY) &&
      (file_mode != O_RDWR)) {
    // invalid mode
    return -1;
  }
  return file_open(file_mode, file_path);
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
