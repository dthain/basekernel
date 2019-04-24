#ifndef PIPE_H
#define PIPE_H

#include "kernel/types.h"

struct pipe *pipe_create();
struct pipe *pipe_addref( struct pipe *p );
void pipe_delete(struct pipe *p);
void pipe_flush(struct pipe *p);
int pipe_set_blocking(struct pipe *p, int b);

int pipe_write(struct pipe *p, char *buffer, int size);
int pipe_read(struct pipe *p, char *buffer, int size);
int pipe_read_nonblock(struct pipe *p, char *buffer, int size);
int pipe_size( struct pipe *p);

#endif
