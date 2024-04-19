// header file for named pipe

#ifndef NAMED_PIPE_H
#define NAMED_PIPE_H

#include "fs.h"
#include "pipe.h"

#define MAX_NAMED_PIPES 100

struct named_pipe {
    struct pipe *base_pipe; // embed basic pipe mechanisms
    char *path; // filesystem path for named pipe
};

// mapping between filesystem and ipc mechanisms
struct named_pipe_mapping {
    struct named_pipe *named_pipe;
    struct fs_dirent *file;
};

int named_pipe_create(const char *fname); // create a named pipe (FIFO) at a specified file path within the filesystem
int named_pipe_open(const char *fname, struct named_pipe **np_out); // open an existing named pipe (FIFO) for reading and writing
int named_pipe_delete(const char *fname); // delete a named pipe (FIFO)

#endif