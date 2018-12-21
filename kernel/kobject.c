/*
 * Copyright (C) 2018 The University of Notre Dame This software is
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

static struct kobject *kobject_init()
{
	struct kobject *k = kmalloc(sizeof(*k));
	k->refcount = 1;
	k->offset = 0;
	k->intent = 0;
	return k;
}

struct kobject *kobject_create_file(struct fs_file *f)
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

int kobject_read(struct kobject *kobject, void *buffer, int size)
{
	int actual = 0;

	switch (kobject->type) {
	case KOBJECT_FILE:
		actual = fs_file_read(kobject->data.file, (char *) buffer, (uint32_t) size, kobject->offset);
		break;
	case KOBJECT_DIR:
		actual = fs_dirent_readdir(kobject->data.dir, (char *)buffer, size );
		break;
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

	if(actual > 0) kobject->offset += actual;

	return actual;
}

int kobject_read_nonblock(struct kobject *kobject, void *buffer, int size)
{
	switch (kobject->type) {
	case KOBJECT_DEVICE:
		return device_read_nonblock(kobject->data.device, buffer, size / device_block_size(kobject->data.device), 0);
	case KOBJECT_PIPE:
		return pipe_read_nonblock(kobject->data.pipe, buffer, size);
	default:
		return kobject_read(kobject,buffer,size);
	}
	return 0;
}

int kobject_write(struct kobject *kobject, void *buffer, int size)
{
	switch (kobject->type) {
	case KOBJECT_GRAPHICS:
		return graphics_write(kobject->data.graphics, (struct graphics_command *) buffer);
	case KOBJECT_CONSOLE:
		return console_write(kobject->data.console, buffer, size );
	case KOBJECT_FILE:{
			int actual = fs_file_write(kobject->data.file, (char *) buffer, (uint32_t) size, kobject->offset);
			if(actual > 0)
				kobject->offset += actual;
			return actual;
		}
	case KOBJECT_DEVICE:
		return device_write(kobject->data.device, buffer, size / device_block_size(kobject->data.device), 0);
	case KOBJECT_PIPE:
		return pipe_write(kobject->data.pipe, buffer, size);
	default:
		return 0;
	}
	return 0;
}

int kobject_copy( struct kobject *ksrc, struct kobject *kdst )
{
	struct fs_dirent *src, *dst;

	if(ksrc->type==KOBJECT_FILE) {
		src = ksrc->data.file->d;
	} else if(ksrc->type==KOBJECT_DIR) {
		src = ksrc->data.dir;
	} else {
		return KERROR_NOT_A_FILE;
	}

	if(kdst->type==KOBJECT_DIR) {
		dst = kdst->data.dir;
	} else {
		return KERROR_NOT_A_DIRECTORY;
	}

	return fs_dirent_copy(src,dst);
}

int kobject_close(struct kobject *kobject)
{
	kobject->refcount--;

	if(kobject->refcount<1) {
		switch (kobject->type) {
		case KOBJECT_GRAPHICS:
			// XXX delete graphics object?
			break;
		case KOBJECT_CONSOLE:
			console_delete(kobject->data.console);
			break;
		case KOBJECT_FILE:
			fs_file_close(kobject->data.file);
			break;
		case KOBJECT_DEVICE:
			// XXX add device close once branch is merged
			//device_close(kobject->data.device);
			break;
		case KOBJECT_PIPE:
			pipe_close(kobject->data.pipe);
			break;
		default:
			break;
		}
		kfree(kobject);
		return 0;
	} else if(kobject->refcount == 1) {
		switch (kobject->type) {
		case KOBJECT_PIPE:
			pipe_flush(kobject->data.pipe);
			return 0;
		default:
			return 0;
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
			dims[0] = fs_file_size(kobject->data.file);
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
			dims[0] = PIPE_SIZE;
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

int kobject_set_intent(struct kobject *kobject, char *new_intent)
{
	if(kobject->intent != 0) {
		kfree(kobject->intent);
	}
	kobject->intent = kmalloc(strlen(new_intent) * sizeof(char));
	strcpy(kobject->intent, new_intent);
	return 1;
}

int kobject_get_intent(struct kobject *kobject, char *buffer, int buffer_size)
{
	if(kobject->intent != 0) {
		strcpy(buffer, kobject->intent);
		return 1;
	}
	return 0;
}

int kobject_dir_lookup( struct kobject *kobject, const char *name, struct fs_dirent **dir )
{
	if(kobject->type==KOBJECT_DIR) {
		*dir = fs_dirent_namei(kobject->data.dir,name);
		if(*dir) {
			return 0;
		} else {
			return KERROR_NOT_FOUND;
		}
	} else {
		return KERROR_NOT_IMPLEMENTED;
	}
	return 0;
}

int kobject_dir_create( struct kobject *kobject, const char *name, struct fs_dirent **dir )
{
	if(kobject->type==KOBJECT_DIR) {
		// XXX mkdir should return the newly created dirent.
		int r = fs_dirent_mkdir(kobject->data.dir,name);
		if(r<0) return r;

		*dir = fs_dirent_namei(kobject->data.dir,name);
		if(!*dir) return KERROR_NOT_FOUND;

		return 0;
	} else {
		return KERROR_NOT_IMPLEMENTED;
	}
	return 0;
}

int kobject_dir_delete( struct kobject *kobject, const char *name )
{
	if(kobject->type==KOBJECT_DIR) {
		return fs_dirent_rmdir(kobject->data.dir,name);
	} else {
		return KERROR_NOT_IMPLEMENTED;
	}
	return 0;
}


