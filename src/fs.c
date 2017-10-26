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
	const struct fs_volume_ops *ops = v->ops;
	if (ops->umount)
		return ops->umount(v);
	return -1;
}

struct dirent *fs_root(struct volume *v)
{
	const struct fs_volume_ops *ops = v->ops;
	if (ops->root)
		return ops->root(v);
	return 0;
}

int fs_readdir(struct dirent *d, char *buffer, int buffer_length)
{
	const struct fs_dirent_ops *ops = d->ops;
	if (ops->readdir)
		return ops->readdir(d, buffer, buffer_length);
	return -1;
}

static struct dirent *fs_lookup(struct dirent *d, const char *name)
{
	const struct fs_dirent_ops *ops = d->ops;
	if (ops->lookup)
		return ops->lookup(d, name);
	return 0;
}

struct dirent *fs_namei( struct dirent *d, const char *path )
{
	char *lpath = kmalloc(strlen(path)+1);
	strcpy(lpath,path);

	char *part = strtok(lpath,"/");
	while(part) {
		d = fs_lookup(d,part);
		if(!d) break;

		part = strtok(0,"/");
	}
	kfree(lpath);
	return d;
}

int fs_dirent_close(struct dirent *d)
{
	const struct fs_dirent_ops *ops = d->ops;
	if (ops->close)
		return ops->close(d);
	return -1;
}

int fs_close(struct file *f)
{
	const struct fs_file_ops *ops = f->ops;
	if (ops->close)
		return ops->close(f);
	return -1;
}

int fs_read(struct file *f, char *buffer, uint32_t n)
{
	const struct fs_file_ops *ops = f->ops;
	if (ops->read)
	{
		return ops->read(f, buffer, n);
	}
	return -1;
}

struct file *fs_open(struct dirent *d, uint8_t mode)
{
	const struct fs_dirent_ops *ops = d->ops;
	if (ops->open)
	{
		return ops->open(d, mode);
	}
	return 0;
}
