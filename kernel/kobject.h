/*
 * Copyright (C) 2018 The University of Notre Dame This software is
 * distributed under the GNU General Public License. See the file LICENSE
 * for details. 
 */

#ifndef KOBJECT_H
#define KOBJECT_H

#include "kernel/types.h"

#include "fs.h"
#include "device.h"
#include "graphics.h"
#include "console.h"
#include "pipe.h"

struct kobject {
	union {
		struct device *device;
		struct fs_file *file;
		struct fs_dirent *dir;
		struct graphics *graphics;
		struct console *console;
		struct pipe *pipe;
	} data;
	kobject_type_t type;
	int refcount;
	int offset;
	char *intent;
};

struct kobject *kobject_create_file(struct fs_file *f);
struct kobject *kobject_create_dir(struct fs_dirent *d);
struct kobject *kobject_create_device(struct device *d);
struct kobject *kobject_create_graphics(struct graphics *g);
struct kobject *kobject_create_console(struct console *c);
struct kobject *kobject_create_pipe(struct pipe *p);

struct kobject *kobject_create_graphics_from_graphics( struct kobject *k, int x, int y, int w, int h );
struct kobject *kobject_create_console_from_graphics( struct kobject *k );

struct kobject *kobject_addref(struct kobject *k);

int kobject_read(struct kobject *kobject, void *buffer, int size);
int kobject_read_nonblock(struct kobject *kobject, void *buffer, int size);
int kobject_write(struct kobject *kobject, void *buffer, int size);
int kobject_size(struct kobject *kobject, int *dimensions, int n);
int kobject_copy( struct kobject *ksrc, struct kobject *kdst );
int kobject_close(struct kobject *kobject);

int kobject_dir_lookup( struct kobject *k, const char *name, struct fs_dirent **d );
int kobject_dir_create( struct kobject *k, const char *name, struct fs_dirent **d );
int kobject_dir_delete( struct kobject *k, const char *name );

int kobject_set_blocking(struct kobject *kobject, int b);
int kobject_get_type(struct kobject *kobject);
int kobject_set_intent(struct kobject *kobject, char *new_intent);
int kobject_get_intent(struct kobject *kobject, char *buffer, int buffer_size);

#endif
