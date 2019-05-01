/*
 * Copyright (C) 2016-2019 The University of Notre Dame This software is
 * distributed under the GNU General Public License. See the file LICENSE
 * for details.
 */

#include "console.h"
#include "kobject.h"
#include "kmalloc.h"

#include "device.h"
#include "fs.h"
#include "graphics.h"
#include "console.h"
#include "pipe.h"

#include "library/string.h"
#include "kernel/error.h"
#include "kernel/stats.h"

static struct kobject *kobject_init()
{
	struct kobject *k = kmalloc(sizeof(*k));
	k->refcount = 1;
	k->offset = 0;
	k->tag = 0;
	k->stats = (const struct object_stats) {0};
	return k;
}

struct kobject *kobject_create_file(struct fs_dirent *f)
{
	struct kobject *k = kobject_init();
	k->type = KOBJECT_FILE;
	k->data.file = f;
	return k;
}

struct kobject *kobject_create_dir( struct fs_dirent *d )
{
	struct kobject *k = kobject_init();
	k->type = KOBJECT_DIR;
	k->data.dir = d;
	return k;
}

struct kobject *kobject_create_device(struct device *d)
{
	struct kobject *k = kobject_init();
	k->type = KOBJECT_DEVICE;
	k->data.device = d;
	return k;
}

struct kobject *kobject_create_graphics(struct graphics *g)
{
	struct kobject *k = kobject_init();
	k->type = KOBJECT_GRAPHICS;
	k->data.graphics = g;
	return k;
}

struct kobject *kobject_create_console(struct console *c)
{
	struct kobject *k = kobject_init();
	k->type = KOBJECT_CONSOLE;
	k->data.console = c;
	return k;
}

struct kobject *kobject_create_pipe(struct pipe *p)
{
	struct kobject *k = kobject_init();
	k->type = KOBJECT_PIPE;
	k->data.pipe = p;
	return k;
}

struct kobject *kobject_addref(struct kobject *k)
{
	k->refcount++;
	return k;
}

struct kobject *kobject_create_graphics_from_graphics( struct kobject *k, int x, int y, int w, int h )
{
	if(k->type!=KOBJECT_GRAPHICS) return 0;

	struct graphics *g = graphics_create(k->data.graphics);
	if(graphics_clip(g,x,y,w,h)) {
		return kobject_create_graphics(g);
	} else {
		graphics_delete(g);
		return 0;
	}
}

struct kobject *kobject_create_console_from_graphics( struct kobject *k )
{
	if(k->type!=KOBJECT_GRAPHICS) return 0;
	struct console *c = console_create(k->data.graphics);
	if(!c) return 0;
	return kobject_create_console(c);
}

struct kobject * kobject_create_file_from_dir( struct kobject *kobject, const char *name )
{
	if(kobject->type==KOBJECT_DIR) {
		struct fs_dirent *d = fs_dirent_mkfile(kobject->data.dir,name);
		if(d) {
			return kobject_create_file(d);
		} else {
			return 0;
		}
	} else {
		return 0;
		// XXX KERROR_NOT_IMPLEMENTED;
	}
	return 0;
}

struct kobject * kobject_create_dir_from_dir( struct kobject *kobject, const char *name )
{
	if(kobject->type==KOBJECT_DIR) {
		struct fs_dirent *d = fs_dirent_mkdir(kobject->data.dir,name);
		if(d) {
			return kobject_create_dir(d);
		} else {
			return 0;
		}
	} else {
		return 0;
		// XXX KERROR_NOT_IMPLEMENTED;
	}
	return 0;
}

int kobject_read(struct kobject *kobject, void *buffer, int size)
{
	int actual = 0;

	switch (kobject->type) {
	case KOBJECT_FILE:
		actual = fs_dirent_read(kobject->data.file, (char *) buffer, (uint32_t) size, kobject->offset);
		break;
	case KOBJECT_DIR:
		return KERROR_INVALID_REQUEST;
	case KOBJECT_DEVICE:
		actual = device_read(kobject->data.device, buffer, size / device_block_size(kobject->data.device), 0);
		break;
	case KOBJECT_PIPE:
		actual = pipe_read(kobject->data.pipe, buffer, size);
		break;
	default:
		actual = 0;
		break;
	}

	if(actual > 0) {
			kobject->offset += actual;
			kobject->stats.bytes_read += actual;
			kobject->stats.reads++;
	}

	return actual;
}

int kobject_read_nonblock(struct kobject *kobject, void *buffer, int size)
{
	int actual = 0;

	switch (kobject->type) {
	case KOBJECT_DEVICE:
		actual = device_read_nonblock(kobject->data.device, buffer, size / device_block_size(kobject->data.device), 0);
		break;
	case KOBJECT_PIPE:
		actual = pipe_read_nonblock(kobject->data.pipe, buffer, size);
		break;
	default:
		actual = kobject_read(kobject,buffer,size);
		break;
	}

	if(actual > 0) {
			kobject->offset += actual;
			kobject->stats.bytes_read += actual;
			kobject->stats.reads++;
	}

	return actual;
}

int kobject_write(struct kobject *kobject, void *buffer, int size)
{
	int actual = 0;

	switch (kobject->type) {
	case KOBJECT_GRAPHICS:
		actual = graphics_write(kobject->data.graphics, (struct graphics_command *) buffer);
		break;
	case KOBJECT_CONSOLE:
		actual = console_write(kobject->data.console, buffer, size );
		break;
	case KOBJECT_FILE:
		actual = fs_dirent_write(kobject->data.file, (char *) buffer, (uint32_t) size, kobject->offset);
		if(actual > 0)
			kobject->offset += actual;
		break;
	case KOBJECT_DEVICE:
		actual = device_write(kobject->data.device, buffer, size / device_block_size(kobject->data.device), 0);
		break;
	case KOBJECT_PIPE:
		actual = pipe_write(kobject->data.pipe, buffer, size);
		break;
	default:
		break;
	}

	if (actual > 0) {
		kobject->stats.bytes_written += actual;
		kobject->stats.writes++;
	}

	return actual;
}

int kobject_list(struct kobject *kobject, void *buffer, int size)
{
	if(kobject->type==KOBJECT_DIR) {
		return fs_dirent_list(kobject->data.dir,buffer,size);
	} else {
		return KERROR_NOT_A_DIRECTORY;
	}
}

int kobject_lookup( struct kobject *kobject, const char *name, struct kobject **newobj )
{
	if(kobject->type==KOBJECT_DIR) {
		struct fs_dirent *d;
		d = fs_dirent_traverse(kobject->data.dir,name);
		if(d) {
			if(fs_dirent_isdir(d)) {
				*newobj = kobject_create_dir(d);
			} else {
				*newobj = kobject_create_file(d);
			}
			return 0;
		} else {
			return KERROR_NOT_FOUND;
		}
	} else {
		return KERROR_NOT_IMPLEMENTED;
	}
	return 0;
}

int kobject_copy( struct kobject *ksrc, struct kobject *kdst )
{
	struct fs_dirent *src, *dst;

	if(ksrc->type==KOBJECT_DIR) {
		src = ksrc->data.dir;
	} else {
		return KERROR_NOT_A_FILE;
	}

	if(kdst->type==KOBJECT_DIR) {
		dst = kdst->data.dir;
	} else {
		return KERROR_NOT_A_DIRECTORY;
	}

	return fs_dirent_copy(src,dst,0);
}

int kobject_remove( struct kobject *kobject, const char *name )
{
	if(kobject->type==KOBJECT_DIR) {
		return fs_dirent_remove(kobject->data.dir,name);
	} else {
		return KERROR_NOT_IMPLEMENTED;
	}
	return 0;
}

int kobject_close(struct kobject *kobject)
{
	kobject->refcount--;

	if(kobject->refcount==0) {
		switch (kobject->type) {
		case KOBJECT_GRAPHICS:
			graphics_delete(kobject->data.graphics);
			break;
		case KOBJECT_CONSOLE:
			console_delete(kobject->data.console);
			break;
		case KOBJECT_FILE:
			fs_dirent_close(kobject->data.file);
			break;
		case KOBJECT_DIR:
			fs_dirent_close(kobject->data.dir);
			break;
		case KOBJECT_DEVICE:
			device_close(kobject->data.device);
			break;
		case KOBJECT_PIPE:
			pipe_delete(kobject->data.pipe);
			break;
		default:
			break;
		}
		kfree(kobject);
		return 0;
	} else if(kobject->refcount>1 ) {
		if(kobject->type==KOBJECT_PIPE) {
			pipe_flush(kobject->data.pipe);
		}
	}
	return 0;
}

int kobject_set_blocking(struct kobject *kobject, int b)
{
	switch (kobject->type) {
	case KOBJECT_PIPE:
		return pipe_set_blocking(kobject->data.pipe, b);
	default:
		return 0;
	}
	return 0;
}

int kobject_size(struct kobject *kobject, int *dims, int n)
{
	switch (kobject->type) {
	case KOBJECT_GRAPHICS:
		if(n==2) {
			dims[0] = graphics_width(kobject->data.graphics);
			dims[1] = graphics_height(kobject->data.graphics);
			return 0;
		} else {
			return KERROR_INVALID_REQUEST;
		}
	case KOBJECT_CONSOLE:
		if(n==2) {
			console_size(kobject->data.console,&dims[0],&dims[1]);
			return 0;
		} else {
			return KERROR_INVALID_REQUEST;
		}
	case KOBJECT_FILE:
		if(n==1) {
			dims[0] = fs_dirent_size(kobject->data.file);
			return 0;
		} else {
			return KERROR_INVALID_REQUEST;
		}
	case KOBJECT_DIR:
		if(n==1) {
			dims[0] = fs_dirent_size(kobject->data.dir);
			return 0;
		} else {
			return KERROR_INVALID_REQUEST;
		}
	case KOBJECT_DEVICE:
		if(n==2) {
			dims[0] = device_nblocks(kobject->data.device);
			dims[1] = device_block_size(kobject->data.device);
			return 0;
		} else {
			return KERROR_INVALID_REQUEST;
		}
	case KOBJECT_PIPE:
		if(n==1) {
			dims[0] = pipe_size(kobject->data.pipe);
			return 0;
		} else {
			return KERROR_INVALID_REQUEST;
		}
	}
	return KERROR_INVALID_REQUEST;
}

int kobject_get_type(struct kobject *kobject)
{
	return kobject->type;
}

int kobject_set_tag(struct kobject *kobject, char *new_tag)
{
	if(kobject->tag != 0) {
		kfree(kobject->tag);
	}
	kobject->tag = kmalloc(strlen(new_tag) * sizeof(char));
	strcpy(kobject->tag, new_tag);
	return 1;
}

int kobject_get_tag(struct kobject *kobject, char *buffer, int buffer_size)
{
	if(kobject->tag != 0) {
		strcpy(buffer, kobject->tag);
		return 1;
	}
	return 0;
}


void kobject_get_stats(struct kobject * kobject, void * stats, int object_type)
{
	switch (object_type) {
	case OBJECT_TYPE:
		memcpy(stats, &(kobject->stats), sizeof(struct object_stats));
		break;
	case PIPE_TYPE:
		pipe_get_stats(kobject->data.pipe, (struct pipe_stats *) stats);
		break;
	case FILE_TYPE:
		fs_dirent_get_stats(kobject->data.file, (struct dirent_stats *) stats, FILE_TYPE);
		break;
	case DIR_TYPE:
		fs_dirent_get_stats(kobject->data.dir, (struct dirent_stats *) stats, DIR_TYPE);
		break;
	case DEVICE_TYPE:
		fs_dirent_get_stats(kobject->data.file, (struct dirent_stats *) stats, DEVICE_TYPE);
		break;
	case CONSOLE_TYPE:
		console_get_stats(kobject->data.console, (struct console_stats *) stats, object_type);
		break;
	case GRAPHICS_TYPE:
		console_get_stats(kobject->data.console, (struct console_stats *) stats, object_type);
		break;

	default:
		break;
	}
}
