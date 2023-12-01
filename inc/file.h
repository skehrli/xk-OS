#pragma once

#include <extent.h>
#include <sleeplock.h>

#define PIPE_BUFFER_SIZE 2048

// an abstraction on top of inodes
// allows for an I/O interface for user processes
struct file_info {
  struct inode *node;
  struct pipe *pipe;
  int isPipe;
  uint offset; // how far we've read
  int mode;    // defined in inc/fcntl.h
  uint ref_count;
  char *path;
  uint gfd; // global file descriptor (in global file table)
};


struct pipe {
  uint read_count;
  uint write_count;
  uint read_offset;
  uint write_offset;
  uint notFull;   // condition variable
  uint notEmpty;  // condition variable
  struct spinlock lock;
  char buffer[PIPE_BUFFER_SIZE];
};

// in-memory copy of an inode
struct inode {
  uint dev;  // Device number
  uint inum; // Inode number
  int ref;   // Reference count
  int valid; // Flag for if node is valid
  struct sleeplock lock;

  // copy of disk inode (see fs.h for details)
  short type;
  short devid;
  uint size;
  struct extent data[EXTENTS];
};

// table mapping device ID (devid) to device functions
struct devsw {
  int (*read)(struct inode *, char *, int);
  int (*write)(struct inode *, char *, int);
};

extern struct devsw devsw[];

// Device ids
enum {
  CONSOLE = 1,
};
