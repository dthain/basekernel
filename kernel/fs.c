#include "fs.h"
#include "kmalloc.h"
#include "string.h"
#include "list.h"
#include "memory.h"

struct list l;

static struct fs_file *init_file(struct fs_dirent *d, uint32_t mode)
{
	struct fs_file *res = kmalloc(sizeof(struct fs_file));
	res->size = d->size;
	res->d = d;
	res->private_data = 0;
	res->mode = mode;
	return res;
}

static void delete_file(struct fs_file *f)
{
	kfree(f);
}

int fs_register(struct fs *f)
{
	struct fs *f_final = kmalloc(sizeof(struct fs));
	memcpy(f_final, f, sizeof(struct fs));
	list_push_tail(&l, &f_final->node);
	return 0;
}

struct fs *fs_get(const char *name)
{
	struct list_node *iter = l.head;
	while(iter) {
		struct fs *f = (struct fs *) iter;
		if(!strcmp(name, f->name)) {
			struct fs *ret = kmalloc(sizeof(struct fs));
			memcpy(ret, f, sizeof(struct fs));
			return ret;
		}
		iter = iter->next;
	}
	return 0;
}

struct fs_volume *fs_volume_mount(struct fs *f, uint32_t device_no)
{
	if(f->mount)
		return f->mount(device_no);
	return 0;
}

int fs_volume_umount(struct fs_volume *v)
{
	const struct fs_volume_ops *ops = v->ops;
	if(ops->umount)
		return ops->umount(v);
	return -1;
}

struct fs_dirent *fs_volume_root(struct fs_volume *v)
{
	const struct fs_volume_ops *ops = v->ops;
	if(ops->root) {
		struct fs_dirent *res = ops->root(v);
		res->v = v;
		return res;
	}
	return 0;
}

int fs_dirent_readdir(struct fs_dirent *d, char *buffer, int buffer_length)
{
	const struct fs_dirent_ops *ops = d->ops;
	if(ops->readdir)
		return ops->readdir(d, buffer, buffer_length);
	return -1;
}

static struct fs_dirent *fs_dirent_lookup(struct fs_dirent *d, const char *name)
{
	const struct fs_dirent_ops *ops = d->ops;
	if(ops->lookup) {
		struct fs_dirent *res = ops->lookup(d, name);
		res->v = d->v;
		return res;
	}
	return 0;
}

int fs_dirent_compare(struct fs_dirent *d1, struct fs_dirent *d2, int *result)
{
	const struct fs_dirent_ops *ops = d1->ops;
	if(ops->compare) {
		int ret = ops->compare(d1, d2, result);
		return ret;
	}
	return -1;
}


struct fs_dirent *fs_dirent_namei(struct fs_dirent *d, const char *path)
{
	char *lpath = kmalloc(strlen(path) + 1);
	strcpy(lpath, path);

	char *part = strtok(lpath, "/");
	while(part) {
		d = fs_dirent_lookup(d, part);
		if(!d)
			break;

		part = strtok(0, "/");
	}
	kfree(lpath);
	return d;
}

int fs_dirent_close(struct fs_dirent *d)
{
	const struct fs_dirent_ops *ops = d->ops;
	if(ops->close)
		return ops->close(d);
	return -1;
}

int fs_file_close(struct fs_file *f)
{
	delete_file(f);
	return 0;
}

struct fs_file *fs_file_open(struct fs_dirent *d, uint8_t mode)
{
	return init_file(d, mode);
}

static int fs_file_read_block(struct fs_file *f, char *buffer, uint32_t blocknum)
{
	return f->d->ops->read_block(f->d, buffer, blocknum);
}

int fs_file_read(struct fs_file *file, char *buffer, uint32_t length, uint32_t offset)
{
	int total = 0;
	int bs = file->d->v->block_size;

	if(offset > file->size) {
		return 0;
	}

	if(offset + length > file->size) {
		length = file->size - offset;
	}

	char *temp = memory_alloc_page(0);

	while(length > 0) {

		int blocknum = offset / bs;
		int actual = 0;

		if(offset % bs) {
			actual = fs_file_read_block(file, temp, blocknum);
			if(actual != bs)
				goto failure;
			actual = bs - offset % bs;
			memcpy(buffer, &temp[offset % bs], actual);
		} else if(length >= bs) {
			actual = fs_file_read_block(file, buffer, blocknum);
			if(actual != bs)
				goto failure;
		} else {
			actual = fs_file_read_block(file, temp, blocknum);
			if(actual != bs)
				goto failure;
			actual = length;
			memcpy(buffer, temp, actual);
		}

		buffer += actual;
		length -= actual;
		offset += actual;
		total += actual;
	}

	memory_free_page(temp);
	return total;

      failure:
	memory_free_page(temp);
	if(total == 0)
		return -1;
	return total;
}

int fs_dirent_mkdir(struct fs_dirent *d, const char *name)
{
	const struct fs_dirent_ops *ops = d->ops;
	if(ops->mkdir) {
		return ops->mkdir(d, name);
	}
	return 0;
}

int fs_dirent_mkfile(struct fs_dirent *d, const char *name)
{
	const struct fs_dirent_ops *ops = d->ops;
	if(ops->mkfile) {
		return ops->mkfile(d, name);
	}
	return 0;
}

int fs_dirent_rmdir(struct fs_dirent *d, const char *name)
{
	const struct fs_dirent_ops *ops = d->ops;
	if(ops->rmdir) {
		return ops->rmdir(d, name);
	}
	return 0;
}

int fs_dirent_unlink(struct fs_dirent *d, const char *name)
{
	const struct fs_dirent_ops *ops = d->ops;
	if(ops->unlink) {
		return ops->unlink(d, name);
	}
	return 0;
}

static int fs_file_write_block(struct fs_file *f, const char *buffer, uint32_t blocknum)
{
	return f->d->ops->write_block(f->d, buffer, blocknum);
}

int fs_file_write(struct fs_file *file, const char *buffer, uint32_t length, uint32_t offset)
{
	int total = 0;
	int bs = file->d->v->block_size;

	char *temp = memory_alloc_page(0);

	while(length > 0) {

		int blocknum = offset / bs;
		int actual = 0;

		if(offset % bs) {
			actual = fs_file_read_block(file, temp, blocknum);
			if(actual != bs)
				goto failure;

			actual = bs - offset % bs;
			memcpy(&temp[offset % bs], buffer, actual);

			int wactual = fs_file_write_block(file, temp, blocknum);
			if(wactual != bs)
				goto failure;

		} else if(length >= bs) {
			actual = fs_file_write_block(file, buffer, blocknum);
			if(actual != bs)
				goto failure;
		} else {
			actual = fs_file_read_block(file, temp, blocknum);
			if(actual != bs)
				goto failure;

			actual = length;
			memcpy(temp, buffer, actual);

			int wactual = fs_file_write_block(file, temp, blocknum);
			if(wactual != bs)
				goto failure;
		}

		buffer += actual;
		length -= actual;
		offset += actual;
		total += actual;
	}

	memory_free_page(temp);
	return total;

      failure:
	memory_free_page(temp);
	if(total == 0)
		return -1;
	return total;
}

int fs_mkfs(struct fs *f, uint32_t device_no)
{
	if(f->mkfs) {
		return f->mkfs(device_no);
	}
	return 0;
}
