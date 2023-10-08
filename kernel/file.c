//
// File descriptors
//

#include <cdefs.h>
#include <defs.h>
#include <file.h>
#include <proc.h>
#include <fs.h>
#include <param.h>
#include <sleeplock.h>
#include <spinlock.h>

struct devsw devsw[NDEV];

// Array of file structs (total of NFILE entries)
struct file_info global_fd_array[NFILE];

int file_read() {
    return -1;
}

int file_write() {
    return -1;
}

/** Open the file specified by path in the given access mode.
 * @param path The path to the file to open.
 * @param access_mode The file access mode.
 * @return The file descriptor index for the current process.
 */ 
int file_open(char *path, int access_mode) {
    struct proc *curr_proc = myproc(); 

    // Finds an open spot in the process file table
    int proc_fd_index = 0;
    while (proc_fd_index < NOFILE && curr_proc->fd_array[proc_fd_index] != NULL)
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
    curr_proc->fd_array[proc_fd_index] = &global_fd_array[global_fd_index];
    
    return proc_fd_index;
}

/** Close the file specified by the given file descriptor.
 * Clean up if this is the last reference.
 * @param fd The file descriptor of the file to close.
 * @return 0 on success, -1 on failure.
 */
int file_close(int fd) {
    struct proc *curr_proc = myproc();

    // Decrement the reference count and clean up if this is the last reference
    if (--curr_proc->fd_array[fd]->refcount <= 0) {
        // Release the inode
        irelease(curr_proc->fd_array[fd]->inode);

        // Deallocate the file_info struct
        *curr_proc->fd_array[fd] = (struct file_info) { 0 };
        curr_proc->fd_array[fd] = NULL;
    }

    return 0;
}