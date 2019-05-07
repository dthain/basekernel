/*
Copyright (C) 2016-2019 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/

#include "kernel/types.h"
#include "pipe.h"
#include "kmalloc.h"
#include "process.h"
#include "list.h"
#include "monitor.h"

#define PIPE_SIZE (1024)

struct pipe {
	char *buffer;
	int read_pos;
	int write_pos;
	int blocking;
	int flushed;
	int refcount;
	struct monitor monitor;
};

struct pipe *pipe_create()
{
	struct pipe *p = kmalloc(sizeof(*p));
	p->buffer = kmalloc(PIPE_SIZE * sizeof(char));
	p->read_pos = 0;
	p->write_pos = 0;
	p->blocking = 1;
	p->flushed = 0;
	p->monitor = (struct monitor) MONITOR_INIT_PROCESS_SAFE;
	p->refcount = 1;
	return p;
}

struct pipe *pipe_addref( struct pipe *p )
{
	p->refcount++;
	return p;
}

void pipe_flush(struct pipe *p)
{
	if(p) {
		p->flushed = 1;
	}
}

void pipe_delete(struct pipe *p)
{
	if(!p) return;

	p->refcount--;
	if(p->refcount==0) {
		if(p->buffer) {
			kfree(p->buffer);
		}
		kfree(p);
	}
}

int pipe_set_blocking(struct pipe *p, int b)
{
	if(p) {
		p->blocking = b;
		return 1;
	}
	return 0;
}

int pipe_write(struct pipe *p, char *buffer, int size)
{
	if(!p || !buffer) {
		return -1;
	}
	int written = 0;

	monitor_lock(&p->monitor);

	if(p->blocking) {
		for(written = 0; written < size; written++) {
			while((p->write_pos + 1) % PIPE_SIZE == p->read_pos) {
				if(p->flushed) goto done;
				monitor_wait(&p->monitor);
			}
			p->buffer[p->write_pos] = buffer[written];
			p->write_pos = (p->write_pos + 1) % PIPE_SIZE;
		}
		monitor_notify_all(&p->monitor);
	} else {
		while(written < size && p->write_pos != (p->read_pos - 1) % PIPE_SIZE) {
			p->buffer[p->write_pos] = buffer[written];
			p->write_pos = (p->write_pos + 1) % PIPE_SIZE;
			written++;
		}
	}

	done:
	p->flushed = 0;
	monitor_unlock(&p->monitor);
	return written;
}

int pipe_read_internal(struct pipe *p, char *buffer, int size, int block)
{
	if(!p || !buffer) {
		return -1;
	}
	int read = 0;

	monitor_lock(&p->monitor);

	if(p->blocking) {
		for(read = 0; read < size; read++) {
			while(p->write_pos == p->read_pos) {
				if(p->flushed) goto done;
				if(block == 0) {
					read = -1;
					goto done;
				}
				monitor_wait(&p->monitor);
			}
			buffer[read] = p->buffer[p->read_pos];
			p->read_pos = (p->read_pos + 1) % PIPE_SIZE;
		}
		monitor_notify_all(&p->monitor);
	} else {
		while(read < size && p->read_pos != p->write_pos) {
			buffer[read] = p->buffer[p->read_pos];
			p->read_pos = (p->read_pos + 1) % PIPE_SIZE;
			read++;
		}
	}

	done:
	p->flushed = 0;	
	monitor_unlock(&p->monitor);
	return read;
}

int pipe_read(struct pipe *p, char *buffer, int size)
{
	return pipe_read_internal(p, buffer, size, 1);
}

int pipe_read_nonblock(struct pipe *p, char *buffer, int size)
{
	return pipe_read_internal(p, buffer, size, 0);
}

int pipe_size( struct pipe *p )
{
	return PIPE_SIZE;
}
