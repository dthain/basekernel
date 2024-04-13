#ifndef NAMED_PIPE_H
#define NAMED_PIPE_H

#include "kernel/types.h"

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


