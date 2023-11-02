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
  struct file_info *file = myproc()->files[fd_copy];
  if (file == NULL) {
    // no open file at this descriptor
    return -1;
  }
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

  if(file->isPipe) {
    if(file->pipe == NULL) {
      return -1;
    }
    acquire(&file->pipe->lock);
    if(file->mode==O_RDONLY) file->pipe->read_count++;
    if(file->mode==O_WRONLY) file->pipe->write_count++;
    release(&file->pipe->lock);
    acquire(&file_table_lock);
    my_proc->files[fd] = file;
    release(&file_table_lock);
    return fd;
  }
  if (file->node == NULL)
    return -1;
  acquire(&file_table_lock);
  my_proc->files[fd] = file;
  file->ref_count++;
  release(&file_table_lock);
  return fd;
}

int pipe_write(int fd, char *buf, int nr_bytes) {
  if (nr_bytes > PIPE_BUFFER_SIZE) return -1;
  struct file_info *file = myproc()->files[fd];
  struct pipe *pipe = file->pipe;
  acquire(&pipe->lock);
  if (pipe->read_count==0) {
    release(&pipe->lock);
    return -1;
  }
  while(PIPE_BUFFER_SIZE - pipe->write_offset + pipe->read_offset < nr_bytes) {
    if (pipe->read_count == 0) {
      release(&pipe->lock);
      return -1;
    }
    sleep(&pipe->notFull, &pipe->lock);
  }
  memmove((void *)(pipe->buffer+(pipe->write_offset%PIPE_BUFFER_SIZE)),(void *)buf, nr_bytes);
  pipe->write_offset += nr_bytes;
  wakeup(&pipe->notEmpty);
  release(&pipe->lock);
  return nr_bytes;
}

int file_write(int fd, char *buf, int nr_bytes) {
  struct proc *my_proc = (struct proc *)myproc();
  struct file_info *file = my_proc->files[fd];
  if (file == NULL) {
    return -1;
  }
  if (file->mode == O_RDONLY)
    return -1;
  if(file->isPipe) {
    return pipe_write(fd, buf, nr_bytes);
  }
  int offset = concurrent_writei(file->node, buf, file->offset, nr_bytes);
  acquire(&file_table_lock);
  my_proc->files[fd]->offset += offset;
  release(&file_table_lock);
  return offset;
}


int pipe_read(int fd, char *buf, int nr_bytes) {
  struct file_info *file = myproc()->files[fd];
  struct pipe *pipe = file->pipe;
  acquire(&pipe->lock);
  if(pipe->write_count == 0) {
    nr_bytes = pipe->write_offset - pipe->read_offset;
    goto read;
  }
  while(pipe->write_offset - pipe->read_offset < nr_bytes) {
    if(pipe->write_count == 0)  {
      nr_bytes = pipe->write_offset - pipe->read_offset;
      goto read;
    }
    sleep(&pipe->notEmpty, &pipe->lock);
  }
read:
  memmove((void *)buf,(void *)(pipe->buffer+(pipe->read_offset%PIPE_BUFFER_SIZE)), nr_bytes);
  pipe->read_offset += nr_bytes;
  wakeup(&pipe->notFull);
  release(&pipe->lock);
  return nr_bytes;
}

int file_read(int fd, char *buf, int nr_bytes) {
  struct proc *my_proc = (struct proc *)myproc();
  struct file_info *fi = myproc()->files[fd];
  if (fi == NULL) {
    // no open file at this descriptor
    return -1;
  }
  if (fi->mode == O_WRONLY)
    return -1;
  if(fi->isPipe) {
    return pipe_read(fd, buf, nr_bytes);
  }
  int offset = concurrent_readi(fi->node, buf, fi->offset, nr_bytes);
  acquire(&file_table_lock);
  my_proc->files[fd]->offset += offset;
  release(&file_table_lock);
  return offset;
}

int file_close(int fd) {
  struct proc *my_proc = myproc();
  struct file_info *fi = my_proc->files[fd];

  acquire(&file_table_lock);
  if (fi->isPipe) {
    if (fi->pipe == NULL) {
      return -1;
    }

    acquire(&fi->pipe->lock);
    if (fi->mode == O_RDONLY) {
      if (--fi->pipe->read_count == 0) {
        if (fi->pipe->write_count == 0) {
          release(&fi->pipe->lock);
          kfree((char *) fi->pipe);
        } else {
          wakeup(&fi->pipe->notFull);
          release(&fi->pipe->lock);
        }
        *fi = (struct file_info) { 0 };
      }
    } else {
      if (--fi->pipe->write_count == 0) {
        if (fi->pipe->read_count == 0) {
          release(&fi->pipe->lock);
          kfree((char *) fi->pipe);
        } else {
          wakeup(&fi->pipe->notEmpty);
          release(&fi->pipe->lock);
        }
        *fi = (struct file_info) { 0 };
      }
    }
    if (fi->pipe != NULL) {
      release(&fi->pipe->lock);
    }

  } else {

    // Clean up if this is the last reference to the file_info
    if (--fi->ref_count <= 0) {
        // Release the inode if this is the last reference to it
        if (--fi->node->ref <= 0 && !fi->isPipe) {
            irelease(fi->node);
        }

        *fi = (struct file_info) { 0 };
    }
  }
  release(&file_table_lock);

  // Remove the file_info from the process's file table
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
  for (int i = 0; i < NOFILE; i++) {
    if (my_proc->files[i] == NULL) {
      fd = i;
      break;
    }
  }
  if (fd == -1) {
    return -1;
  }

  int gfd = -1;
  acquire(&file_table_lock);
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
                         .pipe = NULL,
                         .isPipe = 0,
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

int pipe(int *fd_arr) {
  struct pipe *pipe;
  if((pipe = (struct pipe *)kalloc()) == 0) {
      return -1;
  }
  pipe->read_count = 1;
  pipe->write_count = 1;
  pipe->read_offset = 0;
  pipe->write_offset = 0;
  initlock(&pipe->lock, "pipelock");
  
  int j = 0;
  for (int i = 0; i < NOFILE; i++) {
    if (myproc()->files[i] == NULL) {
      fd_arr[j++] = i;
      if(j == 2) break;
    }
  }
  if (fd_arr[1] == -1) {
    return -1;
  }

  j = 0;
  int gfd[2] = {-1,-1};
  acquire(&file_table_lock);
  for (int i = 0; i < NFILE; i++) {
    if (file_table[i].ref_count)
      continue;
    gfd[j++] = i;
    if(j == 2) break;
  }
  if (gfd[1] == -1) {
    // no free global file descriptor
    release(&file_table_lock);
    return -1;
  }
  
  struct file_info fi_read = {
    .node = NULL,
    .pipe = pipe,
    .isPipe = 1,
    .offset = 0,
    .mode = O_RDONLY,
    .ref_count = 1,
    .path = NULL,
    .gfd = gfd[0]
    };
  struct file_info fi_write = {
    .node = NULL,
    .pipe = pipe,
    .isPipe = 1,
    .offset = 0,
    .mode = O_WRONLY,
    .ref_count = 1,
    .path = NULL,
    .gfd = gfd[1]
  };
  file_table[gfd[0]] = fi_read;
  file_table[gfd[1]] = fi_write;
  myproc()->files[fd_arr[0]] = &(file_table[gfd[0]]);
  myproc()->files[fd_arr[1]] = &(file_table[gfd[1]]);
  release(&file_table_lock);
  return 0;
}

