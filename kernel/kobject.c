/*
 * Copyright (C) 2018 The University of Notre Dame This software is
 * distributed under the GNU General Public License. See the file LICENSE
 * for details. 
 */

#include "console.h"
#include "kobject.h"
#include "kmalloc.h"
#include "fs.h"
#include "device.h"

struct kobject *kobject_create_file(struct fs_file *f)
{
	struct kobject *k = kmalloc(sizeof(*k));
	k->type = KOBJECT_FILE;
	k->refcount = 1;
	k->data.file = f;
	k->offset = 0;

	return k;
}

struct kobject *kobject_create_device(struct device *d)
{
	struct kobject *k = kmalloc(sizeof(*k));
	k->type = KOBJECT_DEVICE;
	k->refcount = 1;
	k->data.device = d;
	return k;
}

struct kobject *kobject_create_graphics(struct graphics *g)
{
	struct kobject *k = kmalloc(sizeof(*k));
	k->type = KOBJECT_GRAPHICS;
	k->refcount = 1;
	k->data.graphics = g;
	return k;
}

struct kobject *kobject_create_pipe(struct pipe *p)
{
	struct kobject *k = kmalloc(sizeof(*k));
	k->type = KOBJECT_PIPE;
	k->refcount = 1;
	k->data.pipe = p;
	return k;
}

struct kobject *kobject_addref(struct kobject *k)
{
	k->refcount++;
	return k;
}

int kobject_read(struct kobject *kobject, void *buffer, int size)
{
	switch (kobject->type) {
	case KOBJECT_INVALID:
		return 0;
	case KOBJECT_GRAPHICS:
		return 0;
	case KOBJECT_FILE:{
			int actual = fs_file_read(kobject->data.file, (char *) buffer, (uint32_t) size, kobject->offset);
			if(actual > 0)
				kobject->offset += actual;
		}
	case KOBJECT_DEVICE:
		return device_read(kobject->data.device, buffer, size / kobject->data.device->block_size, 0);
	case KOBJECT_PIPE:
		return pipe_read(kobject->data.pipe, buffer, size);
	}
	return 0;
}

int kobject_write(struct kobject *kobject, void *buffer, int size)
{
	switch (kobject->type) {
	case KOBJECT_INVALID:
		return 0;
	case KOBJECT_GRAPHICS:
		return graphics_write(kobject->data.graphics, (struct graphics_command *) buffer);
	case KOBJECT_FILE:{
			int actual = fs_file_write(kobject->data.file, (char *) buffer, (uint32_t) size, kobject->offset);
			if(actual > 0)
				kobject->offset += actual;
		}
	case KOBJECT_DEVICE:
		return device_write(kobject->data.device, buffer, size / kobject->data.device->block_size, 0);
	case KOBJECT_PIPE:
		return pipe_write(kobject->data.pipe, buffer, size);
	}
	return 0;
}

int kobject_close(struct kobject *kobject)
{
	int ret;
	if(--kobject->refcount <= 0) {
		switch (kobject->type) {
		case KOBJECT_INVALID:
			return 0;
		case KOBJECT_GRAPHICS:
			return 0;
		case KOBJECT_FILE:
			ret = fs_file_close(kobject->data.file);
			kfree(kobject);
			return ret;
		case KOBJECT_DEVICE:
			return 0;
		case KOBJECT_PIPE:
			pipe_close(kobject->data.pipe);
			return 1;
		}
	} else if(kobject->refcount == 1) {
		switch (kobject->type) {
		case KOBJECT_INVALID:
			return 0;
		case KOBJECT_GRAPHICS:
			return 0;
		case KOBJECT_FILE:
			return 0;
		case KOBJECT_DEVICE:
			return 0;
		case KOBJECT_PIPE:
			pipe_flush(kobject->data.pipe);
			return 0;
		}
	}
	return 0;
}

int kobject_set_blocking(struct kobject *kobject, int b)
{
	switch (kobject->type) {
	case KOBJECT_INVALID:
		return 0;
	case KOBJECT_GRAPHICS:
		return 0;
	case KOBJECT_FILE:
		return 0;
	case KOBJECT_DEVICE:
		return 0;
	case KOBJECT_PIPE:
		return pipe_set_blocking(kobject->data.pipe, b);
	}
	return 0;
}

int kobject_get_type(struct kobject *kobject)
{
	return kobject->type;
}
