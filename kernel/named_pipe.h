#ifndef NAMED_PIPE_H
#define NAMED_PIPE_H

#include "kernel/types.h"
#include "fs.h"


// need a way to manage the state of each Named PIPE. 
//This might include the file descriptor, buffer, read/write locks, and reference counts.


// Named PIPE structure


// typedef struct named_pipe {
//     char name[256];                   // Name of the PIPE, used for file system entry
//     struct file *file;                // File structure representing the PIPE
//     char buffer[NAMED_PIPE_BUFFER_SIZE]; // Buffer for the PIPE data
//     int read_pos;                     // Current read position in the buffer
//     int write_pos;                    // Current write position in the buffer
//     int open_handles;                 // Number of open handles to the PIPE (for synchronization)
//     mutex_t lock;                     // Mutex for synchronizing access to the PIPE
// } named_pipe;


struct named_pipe *named_pipe_create(char *fname);
struct named_pipe *named_pipe_addref(struct named_pipe *p);
void named_pipe_delete(struct named_pipe *p);
void named_pipe_flush(struct named_pipe *p);

int named_pipe_write(struct named_pipe *p, char *buffer, int size);
int named_pipe_write_nonblock(struct named_pipe *p, char *buffer, int size);
int named_pipe_read(struct named_pipe *p, char *buffer, int size);
int named_pipe_read_nonblock(struct named_pipe *p, char *buffer, int size);
int named_pipe_size(struct named_pipe *p);

#endif


