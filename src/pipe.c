/*
 * Copyright (C) 2018 The University of Notre Dame This software is
 * distributed under the GNU General Public License. See the file LICENSE
 * for details. 
 */

#include "kerneltypes.h"
#include "pipe.h"
#include "kmalloc.h"


struct pipe *pipe_open() {
    struct pipe *p = kmalloc(sizeof(*p));
    p->buffer = kmalloc(PIPE_SIZE*sizeof(char));
    p->read_pos = 0;
    p->write_pos = 0;
    return p;
}

void pipe_close(struct pipe *p) {
    if (p) {
        if (p->buffer) {
            kfree(p->buffer);
        }
        kfree(p);
    }
}

int pipe_write(struct pipe *p, char *buffer, int size) {
    if (!p || !buffer) {
        return -1;
    }
    int written = 0;
    while (written < size && p->write_pos != (p->read_pos-1)%PIPE_SIZE) {
        p->buffer[p->write_pos] = buffer[written];
        p->write_pos = (p->write_pos+1) % PIPE_SIZE;
        written++;
    }
    return written;
}

int pipe_read(struct pipe *p, char *buffer, int size) {
    if (!p || !buffer) {
        return -1;
    }
    int read = 0;
    while (read < size && p->read_pos != p->write_pos) {
        buffer[read] = p->buffer[p->read_pos];
        p->read_pos = (p->read_pos+1) % PIPE_SIZE;
        read++;
    }
    return read;
}
