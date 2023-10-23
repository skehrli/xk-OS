//
// File descriptors
//

#include "../inc/cdefs.h"
#include "../inc/defs.h"
#include "../inc/proc.h"
#include <cdefs.h>
#include <defs.h>
#include <fcntl.h>
#include <file.h>
#include <fs.h>
#include <param.h>
#include <proc.h>
#include <sleeplock.h>
#include <spinlock.h>
#include <stat.h>

struct devsw devsw[NDEV];

static struct file_info file_table[NFILE];
struct spinlock file_table_lock;

int file_stat(int fd, struct stat *stat_ptr) {
  struct proc *my_proc = (struct proc *)myproc();
  acquire(&file_table_lock);
  if (my_proc->files[fd] == NULL) {
    // no open file at this descriptor
    release(&file_table_lock);
    return -1;
  }
  if (my_proc->files[fd]->node == NULL) {
    // underlying inode ptr is null
    return -1;
  }
  struct inode *node = my_proc->files[fd]->node;
  release(&file_table_lock);
  concurrent_stati(node, stat_ptr);
  return 0;
}

int file_dup(int fd_copy) {
  struct proc *my_proc = (struct proc *)myproc();
  acquire(&file_table_lock);
  if (my_proc->files[fd_copy] == NULL) {
    // no open file at this descriptor
    release(&file_table_lock);
    return -1;
  }
  if (my_proc->files[fd_copy]->node == NULL)
    return -1;
  release(&file_table_lock);
  int fd = -1;
  for (int i = 0; i < NOFILE; i++) {
    if (my_proc->files[i] == NULL) {
      fd = i;
      break;
    }
  }
  if (fd == -1) {
    return -1;
  }
  acquire(&file_table_lock);
  my_proc->files[fd] = my_proc->files[fd_copy];
  my_proc->files[fd_copy]->ref_count++;
  release(&file_table_lock);
  return fd;
}

int file_write(int fd, char *buf, int nr_bytes) {
  struct proc *my_proc = (struct proc *)myproc();
  acquire(&file_table_lock);
  if (my_proc->files[fd] == NULL) {
    // no open file at this descriptor
    release(&file_table_lock);
    return -1;
  }
  struct file_info *fi = my_proc->files[fd];
  release(&file_table_lock);
  if (fi->mode == O_RDONLY)
    return -1;
  int offset = concurrent_writei(fi->node, buf, fi->offset, nr_bytes);
  acquire(&file_table_lock);
  my_proc->files[fd]->offset += offset;
  release(&file_table_lock);
  return offset;
}

int file_read(int fd, char *buf, int nr_bytes) {
  struct proc *my_proc = (struct proc *)myproc();
  acquire(&file_table_lock);
  if (my_proc->files[fd] == NULL) {
    // no open file at this descriptor
    release(&file_table_lock);
    return -1;
  }
  struct file_info *fi = my_proc->files[fd];
  release(&file_table_lock);
  if (fi->mode == O_WRONLY)
    return -1;
  int offset = concurrent_readi(fi->node, buf, fi->offset, nr_bytes);
  acquire(&file_table_lock);
  my_proc->files[fd]->offset += offset;
  release(&file_table_lock);
  return offset;
}

int file_close(int fd) {
  struct proc *my_proc = (struct proc *)myproc();
  acquire(&file_table_lock);
  if (my_proc->files[fd] == NULL) {
    // no open file at this descriptor
    release(&file_table_lock);
    return -1;
  }
  if (my_proc->files[fd]->ref_count > 1)
    my_proc->files[fd]->ref_count--;
  else {
    int gfd = my_proc->files[fd]->gfd;
    int inode_refcount = --file_table[gfd].node->ref;
    if (inode_refcount == 0) {
      irelease(file_table[gfd].node);
    }
    file_table[gfd].node = 0;
    file_table[gfd].offset = 0;
    file_table[gfd].mode = 0;
    file_table[gfd].ref_count = 0;
    file_table[gfd].path = 0;
    file_table[gfd].gfd = 0;
  }
  release(&file_table_lock);
  my_proc->files[fd] = NULL;
  return 0;
}

int file_open(int file_mode, char *file_path) {
  struct proc *my_proc = (struct proc *)myproc();

  struct inode *node = (struct inode *)iopen(file_path);
  if (node == NULL) {
    return -1;
  }
  if ((node->type == T_FILE) && (file_mode != O_RDONLY)) {
    return -1;
  }

  int fd = -1;
  acquire(&file_table_lock);
  for (int i = 0; i < NOFILE; i++) {
    if (my_proc->files[i] == NULL) {
      fd = i;
      break;
    }
  }
  if (fd == -1) {
    release(&file_table_lock);
    return -1;
  }

  int gfd = -1;

  for (int i = 0; i < NFILE; i++) {
    if (file_table[i].ref_count)
      continue;
    gfd = i;
    break;
  }
  if (gfd == -1) {
    // no free global file descriptor
    release(&file_table_lock);
    return -1;
  }
  struct file_info fi = {.node = node,
                         .offset = 0,
                         .mode = file_mode,
                         .ref_count = 1,
                         .path = file_path,
                         .gfd = gfd};
  file_table[gfd] = fi;
  my_proc->files[fd] = &(file_table[gfd]);
  release(&file_table_lock);

  return fd;
}
