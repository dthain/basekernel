#include "fs.h"
#include "kmalloc.h"
#include "kernel/syscall.h"
#include "string.h"
#include "list.h"
#include "memory.h"
#include "process.h"

static struct fs *fs_list = 0;

struct fs_dirent *fs_resolve(const char *path)
{
	if(path[0] == '/') {
		return fs_dirent_namei(current->root_dir, &path[1]);
	} else {
		return fs_dirent_namei(current->current_dir, path);
	}
}

void fs_register(struct fs *f)
{
	f->next = fs_list;
	fs_list = f;
}

struct fs *fs_lookup(const char *name)
{
	struct fs *f;

	for(f = fs_list; f; f = f->next) {
		if(!strcmp(name, f->name)) {
			return f;
		}
	}
	return 0;
}

int fs_mkfs(struct fs *f, uint32_t device_no)
{
	const struct fs_ops *ops = f->ops;
	if(!ops->mkfs)
		return KERROR_NOT_SUPPORTED;
	return f->ops->mkfs(device_no);
}

struct fs_volume *fs_volume_open(struct fs *f, uint32_t device_no)
{
	const struct fs_ops *ops = f->ops;
	if(!ops->mount)
		return 0;
	struct fs_volume *v = f->ops->mount(device_no);
	if(v)
		v->fs = f;
	return v;
}

struct fs_volume *fs_volume_addref(struct fs_volume *v)
{
	v->refcount++;
	return v;
}

int fs_volume_close(struct fs_volume *v)
{
	const struct fs_ops *ops = v->fs->ops;
	if(!ops->umount)
		return KERROR_NOT_SUPPORTED;

	v->refcount--;
	if(v->refcount <= 0)
		return v->fs->ops->umount(v);
	return -1;
}

struct fs_dirent *fs_volume_root(struct fs_volume *v)
{
	const struct fs_ops *ops = v->fs->ops;
	if(!ops->root)
		return 0;

	struct fs_dirent *d = v->fs->ops->root(v);
	d->v = fs_volume_addref(v);
	return d;
}

int fs_dirent_readdir(struct fs_dirent *d, char *buffer, int buffer_length)
{
	const struct fs_ops *ops = d->v->fs->ops;
	if(!ops->readdir)
		return KERROR_NOT_SUPPORTED;
	return ops->readdir(d, buffer, buffer_length);
}

static struct fs_dirent *fs_dirent_lookup(struct fs_dirent *d, const char *name)
{
	const struct fs_ops *ops = d->v->fs->ops;
	if(!ops->lookup)
		return 0;

	struct fs_dirent *r = ops->lookup(d, name);
	r->v = fs_volume_addref(d->v);
	return r;
}

int fs_dirent_compare(struct fs_dirent *d1, struct fs_dirent *d2, int *result)
{
	const struct fs_ops *ops = d1->v->fs->ops;
	if(!ops->compare)
		return KERROR_NOT_SUPPORTED;

	return d1->v->fs->ops->compare(d1, d2, result);
}

struct fs_dirent *fs_dirent_namei(struct fs_dirent *d, const char *path)
{
	if(!d || !path)
		return 0;

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

struct fs_dirent *fs_dirent_addref(struct fs_dirent *d)
{
	d->refcount++;
	return d;
}

int fs_dirent_close(struct fs_dirent *d)
{
	const struct fs_ops *ops = d->v->fs->ops;
	if(!ops->close)
		return KERROR_NOT_SUPPORTED;

	d->refcount--;
	if(d->refcount <= 0) {
		struct fs_volume *v = d->v;
		ops->close(d);
		fs_volume_close(v);
	}

	return 0;
}

struct fs_file *fs_file_open(struct fs_dirent *d, uint8_t mode)
{
	struct fs_file *f = kmalloc(sizeof(*f));
	f->size = d->size;
	f->d = fs_dirent_addref(d);
	f->private_data = 0;
	f->mode = mode;
	f->refcount = 1;
	return f;
}

struct fs_file *fs_file_addref(struct fs_file *f)
{
	f->refcount++;
	return f;
}

int fs_file_close(struct fs_file *f)
{
	if(!f)
		return 0;
	f->refcount--;
	if(f->refcount <= 0) {
		fs_dirent_close(f->d);
		// XXX free private data?
		kfree(f);
	}
	return 0;
}

int fs_file_read(struct fs_file *file, char *buffer, uint32_t length, uint32_t offset)
{
	int total = 0;
	int bs = file->d->v->block_size;

	const struct fs_ops *ops = file->d->v->fs->ops;
	if(!ops->read_block)
		return KERROR_INVALID_REQUEST;

	if(offset > file->size) {
		return 0;
	}

	if(offset + length > file->size) {
		length = file->size - offset;
	}

	char *temp = memory_alloc_page(0);
	if(!temp)
		return -1;

	while(length > 0) {

		int blocknum = offset / bs;
		int actual = 0;

		if(offset % bs) {
			actual = ops->read_block(file->d, temp, blocknum);
			if(actual != bs)
				goto failure;
			actual = MIN(bs - offset % bs, length);
			memcpy(buffer, &temp[offset % bs], actual);
		} else if(length >= bs) {
			actual = ops->read_block(file->d, buffer, blocknum);
			if(actual != bs)
				goto failure;
		} else {
			actual = ops->read_block(file->d, temp, blocknum);
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
	const struct fs_ops *ops = d->v->fs->ops;
	if(!ops->mkdir)
		return 0;
	return ops->mkdir(d, name);
}

int fs_dirent_mkfile(struct fs_dirent *d, const char *name)
{
	const struct fs_ops *ops = d->v->fs->ops;
	if(!ops->mkfile)
		return 0;
	return ops->mkfile(d, name);
}

int fs_dirent_rmdir(struct fs_dirent *d, const char *name)
{
	const struct fs_ops *ops = d->v->fs->ops;
	if(!ops->rmdir)
		return 0;
	return ops->rmdir(d, name);
}

int fs_dirent_unlink(struct fs_dirent *d, const char *name)
{
	const struct fs_ops *ops = d->v->fs->ops;
	if(!ops->unlink)
		return 0;
	return ops->unlink(d, name);
}

int fs_file_write(struct fs_file *file, const char *buffer, uint32_t length, uint32_t offset)
{
	int total = 0;
	int bs = file->d->v->block_size;

	const struct fs_ops *ops = file->d->v->fs->ops;
	if(!ops->write_block || !ops->read_block)
		return KERROR_INVALID_REQUEST;

	char *temp = memory_alloc_page(0);

	// if writing past the (current) end of the file, resize the file first
	if (offset + length > file->size) {
		ops->resize(file->d, offset+length);
	}

	while(length > 0) {

		int blocknum = offset / bs;
		int actual = 0;

		if(offset % bs) {
			actual = ops->read_block(file->d, temp, blocknum);
			if(actual != bs)
				goto failure;

			actual = MIN(bs - offset % bs, length);
			memcpy(&temp[offset % bs], buffer, actual);

			int wactual = ops->write_block(file->d, temp, blocknum);
			if(wactual != bs)
				goto failure;

		} else if(length >= bs) {
			actual = ops->write_block(file->d, buffer, blocknum);
			if(actual != bs)
				goto failure;
		} else {
			actual = ops->read_block(file->d, temp, blocknum);
			if(actual != bs)
				goto failure;

			actual = length;
			memcpy(temp, buffer, actual);

			int wactual = ops->write_block(file->d, temp, blocknum);
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

int fs_file_get_dimensions(struct fs_file *f, int * dims, int n)
{
	if (n <= 0)
		return 0;

	dims[0] = f->size;

	return 1;
}

int fs_dirent_copy(struct fs_dirent *src, struct fs_dirent *dst) {
	char *buffer = kmalloc(PAGE_SIZE);
	memset(buffer,0,PAGE_SIZE);
	printf("Reading directory... ");
	int length = fs_dirent_readdir(src, buffer, 4096);
	printf("Done.\n");
	if (length <= 0) {
		return length;
	}
	char *name = buffer;
	while (name && (name - buffer) < length) {
		if (strcmp(name,".") == 0 || (strcmp(name, "..") == 0)) {
			name += strlen(name) + 1;
			continue;
		}
		printf("Copying %s...\n", name);
		struct fs_dirent *new_src, *new_dst;
		new_src = fs_dirent_lookup(src, name);
		char temp[1];
		if (fs_dirent_readdir(new_src, temp, 1) == KERROR_NOT_A_DIRECTORY) { //file
			fs_dirent_mkfile(dst, name);
			new_dst = fs_dirent_lookup(dst, name);
			struct fs_file *src_file = fs_file_open(new_src, FS_FILE_READ);
			struct fs_file *dst_file = fs_file_open(new_dst, FS_FILE_WRITE);
			char * filebuf = kmalloc(src_file->size);
			fs_file_read(src_file, filebuf,src_file->size,0);
			fs_file_write(dst_file, filebuf, src_file->size, 0);
			kfree(filebuf);
			fs_file_close(src_file);
			fs_file_close(dst_file);
		} else { // directory
			fs_dirent_mkdir(dst,name);
			struct fs_dirent *new_dst = fs_dirent_lookup(dst, name);
			fs_dirent_copy(new_src, new_dst);
		}
		fs_dirent_close(new_dst);
		fs_dirent_close(new_src);
		printf("Done.\n");

		name += strlen(name) + 1;
	}
	kfree(buffer);
	return 0;
}
