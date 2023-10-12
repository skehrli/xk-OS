//
// File descriptors
//

#include <cdefs.h>
#include <defs.h>
#include <fcntl.h>
#include <file.h>
#include <proc.h>
#include <fs.h>
#include <param.h>
#include <sleeplock.h>
#include <spinlock.h>
#include <stat.h>

struct devsw devsw[NDEV];

// Array of file structs (total of NFILE entries)
struct file_info global_fd_array[NFILE];

/** Read n bytes from the file specified by the given file descriptor and change the file offset.
 * @param fd The file descriptor of the file to read from.
 * @param buf The buffer to read into.
 * @param n The number of bytes to read.
 * @return The number of bytes read.
 */
int file_read(int fd, char *buf, int n) {
    struct proc *my_proc = myproc();
    struct file_info *file_info_ptr = my_proc->fd_array[fd];

    // Error if the file is write-only
    if (file_info_ptr->access_mode == O_WRONLY) {
        return -1;
    }

    /* TODO: Fewer than arg2 bytes might be read due to these conditions:
    * If the current position + arg2 is beyond the end of the file.
    * If this is a pipe or console device and fewer than arg2 bytes are available 
    * If this is a pipe and the other end of the pipe has been closed.
    */
    int bytes_read = concurrent_readi(file_info_ptr->inode, buf, file_info_ptr->offset, n);

    // Update the file offset
    file_info_ptr->offset += bytes_read;

    return bytes_read;
}

int file_write(int fd, char *buf, int n) {
    struct proc *my_proc = myproc();
    struct file_info *file_info_ptr = my_proc->fd_array[fd];

    // Error if the file is read-only
    if (file_info_ptr->access_mode == O_RDONLY) {
        return -1;
    }

    // TODO: If the full write cannot be completed, write as many as possible before returning with that number of bytes.
    int bytes_written = concurrent_writei(file_info_ptr->inode, buf, file_info_ptr->offset, n);
    my_proc->fd_array[fd]->offset += bytes_written;
    return bytes_written;
}

/** Open the file specified by path in the given access mode.
 * @param path The path to the file to open.
 * @param access_mode The file access mode.
 * @return The file descriptor index for the current process.
 */ 
int file_open(char *path, int access_mode) {
    struct proc *my_proc = myproc();

    if (access_mode == T_DEV) {
        return -1;
    }

    // Finds an open spot in the process file table
    int proc_fd_index = 0;
    while (proc_fd_index < NOFILE && my_proc->fd_array[proc_fd_index] != NULL)
    {
        ++proc_fd_index;   
    }

    // Error if the process file table is full
    if (proc_fd_index == NOFILE) {
        return -1;
    }

    // Finds an open entry in the global file table
    int global_fd_index = 0;
    while (global_fd_index < NFILE && global_fd_array[global_fd_index].refcount != 0)
    {
        ++global_fd_index;   
    }

    struct inode *inode_ptr = iopen(path);

    global_fd_array[global_fd_index] = (struct file_info){ .inode=inode_ptr, .offset=0, .access_mode=access_mode, .refcount=1 };
    my_proc->fd_array[proc_fd_index] = &global_fd_array[global_fd_index];
    
    return proc_fd_index;
}

/** Close the file specified by the given file descriptor.
 * Clean up if this is the last reference.
 * @param fd The file descriptor of the file to close.
 * @return 0 on success, -1 on failure.
 */
int file_close(int fd) {
    struct proc *my_proc = myproc();

    // Decrement the reference count for the file_info pointed by the file descriptor
    int file_info_refcount = --my_proc->fd_array[fd]->refcount;
    
    // Clean up if this is the last reference to the file_info
    if (file_info_refcount <= 0) {
        // Decrement the reference count for the inode
        int inode_refcount = --my_proc->fd_array[fd]->inode->ref;
        // Release the inode if this is the last reference to it
        if (inode_refcount <= 0) {
            irelease(my_proc->fd_array[fd]->inode);
        }

        // Deallocate the file_info struct
        *my_proc->fd_array[fd] = (struct file_info) { 0 };
    }

    // Remove the file_info from the process file table
    my_proc->fd_array[fd] = NULL;
    return 0;
}

/** Duplicate the file descriptor.
 * @param fd_copy The file descriptor of the file to duplicate.
 * @return The new file descriptor index for the current process.
 */
int file_dup(int fd_copy) {
    struct proc *my_proc = myproc();

    // Finds an open spot in the process file table
    int fd = 0;
    while (fd < NOFILE && my_proc->fd_array[fd] != NULL) {
        ++fd;   
    }

    // Error if the process file table is full
    if (fd == NOFILE) {
        return -1;
    }

    struct file_info *file_info_ptr = my_proc->fd_array[fd_copy];
    // Add the file_info to the process file table
    my_proc->fd_array[fd] = file_info_ptr;
    // Increment the reference count
    file_info_ptr->refcount++;

    return fd;
}

/** Return statistics to the user about a file.
 * @param fd The file descriptor of the file to get statistics for.
 * @param stat_ptr The struct stat pointer to populate.
 * @return 0 on success, -1 on failure.
*/
int file_stat(int fd, struct stat *stat_ptr) {
  struct proc *my_proc = myproc();
  
  if (my_proc->fd_array[fd]->inode == NULL) {
    // underlying inode ptr is null
    return -1;
  }

  concurrent_stati(my_proc->fd_array[fd]->inode, stat_ptr);
  return 0;
}
