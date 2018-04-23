/*
 * Copyright (C) 2018 The University of Notre Dame This software is
 * distributed under the GNU General Public License. See the file LICENSE
 * for details. 
 */

#ifndef PIPE_H
#define PIPE_H

#include "kerneltypes.h"

#define PIPE_SIZE (1024)

struct pipe {
    char *buffer;
    int read_pos;
    int write_pos;
};

struct pipe *pipe_open();
void pipe_close(struct pipe *p);

int pipe_write(struct pipe *p, char *buffer, int size);
int pipe_read(struct pipe *p, char *buffer, int size);

#endif
