/*
 * Copyright (C) 2018 The University of Notre Dame This software is
 * distributed under the GNU General Public License. See the file LICENSE
 * for details. 
 */

#ifndef PIPE_H
#define PIPE_H

#include "kernel/types.h"
#include "list.h"

#define PIPE_SIZE (1024)

struct pipe {
	char *buffer;
	int read_pos;
	int write_pos;
	int blocking;
	int flushed;
	struct list queue;
};

struct pipe *pipe_open();
void pipe_close(struct pipe *p);
void pipe_flush(struct pipe *p);
int pipe_set_blocking(struct pipe *p, int b);

int pipe_write(struct pipe *p, char *buffer, int size);
int pipe_read(struct pipe *p, char *buffer, int size);
int pipe_read_nonblock(struct pipe *p, char *buffer, int size);

#endif
