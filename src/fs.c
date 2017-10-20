#include "fs.h"
#include "kmalloc.h"
#include "string.h"
#include "list.h"

struct list l;

int fs_register(struct fs *f) {
	list_push_tail(&l, &f->node);
	return 0;
}

struct fs *fs_get(const char *name) {
	struct list_node *iter = l.head;
	while (iter) {
		struct fs *f = (struct fs*) iter;
		if (!strcmp(name, f->name)) {
			struct fs *ret = kmalloc(sizeof(struct fs));
			memcpy(ret, f, sizeof(struct fs));
			return ret;
		}
		iter = iter->next;
	}
	return 0;
}

struct volume *fs_mount(struct fs *f, uint32_t device_no)
{
	if (f->mount)
		return f->mount(device_no);
	return 0;
}

int fs_umount(struct volume *v)
{
	if (v->umount)
		return v->umount(v);
	return -1;
}

struct dirent *fs_root(struct volume *v)
{
	if (v->root)
		return v->root(v);
	return 0;
}

int fs_readdir(struct dirent *d, char *buffer, int buffer_length)
{
	if (d->readdir)
		return d->readdir(d, buffer, buffer_length);
	return -1;
}

struct dirent *fs_lookup(struct dirent *d, const char *name)
{
	if (d->lookup)
		return d->lookup(d, name);
	return 0;
}

int fs_dirent_close(struct dirent *d)
{
	if (d->close)
		return d->close(d);
	return -1;
}

int fs_close(struct file *f)
{
	if (f->close)
		return f->close(f);
	return -1;
}

int fs_read(struct file *f, char *buffer, uint32_t n)
{
	if (f->read)
	{
		return f->read(f, buffer, n);
	}
	return -1;
}

struct file *fs_open(struct dirent *d, uint8_t mode)
{
	if (d->open)
	{
		return d->open(d, mode);
	}
	return 0;
}
