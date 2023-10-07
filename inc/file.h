#pragma once

#include <extent.h>
#include <sleeplock.h>

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
  struct extent data;
};

// Metadata for an open file
// - the underlying inode
// - current offset of the file (how far have we read)
// - access mode of the open file (identifies the file as readable, writable and so on)
// - reference count of the struct
struct file_info {
  struct inode *inode;
  int offset;
  int access_mode;
  int refcount;
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
