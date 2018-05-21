#include "fs.h"
#include "fs_ops.h"
#include "kmalloc.h"
#include "string.h"
#include "list.h"

struct list l;

struct fs_block_map {
	char *buffer;
	uint32_t block_size;
	uint32_t block;
	uint32_t offset;
	uint32_t read_length;
};

static struct fs_block_map *init_block_map(uint32_t block_size) {
	struct fs_block_map *res = kmalloc(sizeof(struct fs_block_map));
	memset(res, 0, sizeof(struct fs_block_map));
	res->block_size = block_size;
	res->buffer = kmalloc(sizeof(char) * block_size);
	return res;
}

static void delete_block_map(struct fs_block_map *bm) {
	kfree(bm->buffer);
	kfree(bm);
}

static struct fs_file *init_file(struct fs_dirent *d, uint32_t mode) {
	struct fs_file *res = kmalloc(sizeof(struct fs_file));
	struct fs_volume *v = d->v;
	res->sz = d->sz;
	res->d = d;
	res->private_data = init_block_map(v->block_size);
	res->mode = mode;
	return res;
}

static void delete_file(struct fs_file *f) {
	struct fs_block_map *bm = f->private_data;
	delete_block_map(bm);
	kfree(f);
}

int fs_register(struct fs *f) {
	struct fs *f_final = kmalloc(sizeof(struct fs));
	memcpy(f_final, f, sizeof(struct fs));
	list_push_tail(&l, &f_final->node);
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

struct fs_volume *fs_volume_mount(struct fs *f, uint32_t device_no)
{
	if (f->mount)
		return f->mount(device_no);
	return 0;
}

int fs_volume_umount(struct fs_volume *v)
{
	const struct fs_volume_ops *ops = v->ops;
	if (ops->umount)
		return ops->umount(v);
	return -1;
}

struct fs_dirent *fs_volume_root(struct fs_volume *v)
{
	const struct fs_volume_ops *ops = v->ops;
	if (ops->root) {
		struct fs_dirent *res = ops->root(v);
		res->v = v;
		return res;
	}
	return 0;
}

int fs_dirent_readdir(struct fs_dirent *d, char *buffer, int buffer_length)
{
	const struct fs_dirent_ops *ops = d->ops;
	if (ops->readdir)
		return ops->readdir(d, buffer, buffer_length);
	return -1;
}

static struct fs_dirent *fs_dirent_lookup(struct fs_dirent *d, const char *name)
{
	const struct fs_dirent_ops *ops = d->ops;
	if (ops->lookup) {
		struct fs_dirent *res = ops->lookup(d, name);
		res->v = d->v;
		return res;
	}
	return 0;
}

int fs_dirent_compare(struct fs_dirent *d1, struct fs_dirent *d2, int *result)
{
	const struct fs_dirent_ops *ops = d1->ops;
	if (ops->compare) {
		int ret = ops->compare(d1, d2, result);
		return ret;
	}
	return -1;
}


struct fs_dirent *fs_dirent_namei( struct fs_dirent *d, const char *path )
{
	char *lpath = kmalloc(strlen(path)+1);
	strcpy(lpath,path);

	char *part = strtok(lpath,"/");
	while(part) {
		d = fs_dirent_lookup(d,part);
		if(!d) break;

		part = strtok(0,"/");
	}
	kfree(lpath);
	return d;
}

int fs_dirent_close(struct fs_dirent *d)
{
	const struct fs_dirent_ops *ops = d->ops;
	if (ops->close)
		return ops->close(d);
	return -1;
}

int fs_file_close(struct fs_file *f)
{
	delete_file(f);
	return 0;
}

static int fs_file_read_block(struct fs_file *f, char *buffer, uint32_t blocknum)
{
	struct fs_dirent *d = f->d;
	const struct fs_dirent_ops *ops = d->ops;
	if (ops->read_block)
	{
		 return ops->read_block(d, buffer, blocknum);
	}
	return -1;
}

struct fs_file *fs_file_open(struct fs_dirent *d, uint8_t mode)
{
	return init_file(d, mode);
}

int fs_file_read(struct fs_file *f, char *buffer, uint32_t n)
{
	struct fs_block_map *bm = f->private_data;
	uint32_t read = 0;

	if (!(FS_FILE_READ & f->mode))
		return -1;

	while (read < n && bm->block * bm->block_size + (bm->offset % bm->block_size) < f->sz) {
		uint32_t to_read = 0;
		if (bm->offset == bm->read_length) {
			fs_file_read_block(f, bm->buffer, bm->block);
			if (f->sz <= (bm->block + 1) * bm->block_size) {
				bm->read_length = f->sz - bm->block * bm->block_size;
			} else {
				bm->read_length = bm->block_size;
			}
			bm->block++;
			bm->offset = 0;
		}
		to_read = bm->block_size - bm->offset;
		if (n - read < bm->block_size - bm->offset) {
			to_read = n - read;
		}
		memcpy(buffer + read, bm->buffer + bm->offset, to_read);
		bm->offset += to_read;
		read += to_read;
	}
	return read;
}

int fs_dirent_mkdir(struct fs_dirent *d, const char *name)
{
	const struct fs_dirent_ops *ops = d->ops;
	if (ops->mkdir)
	{
		return ops->mkdir(d, name);
	}
	return 0;
}

int fs_dirent_mkfile(struct fs_dirent *d, const char *name)
{
	const struct fs_dirent_ops *ops = d->ops;
	if (ops->mkfile)
	{
		return ops->mkfile(d, name);
	}
	return 0;
}

int fs_dirent_rmdir(struct fs_dirent *d, const char *name)
{
	const struct fs_dirent_ops *ops = d->ops;
	if (ops->rmdir)
	{
		return ops->rmdir(d, name);
	}
	return 0;
}

int fs_dirent_unlink(struct fs_dirent *d, const char *name)
{
	const struct fs_dirent_ops *ops = d->ops;
	if (ops->unlink)
	{
		return ops->unlink(d, name);
	}
	return 0;
}

int fs_file_write(struct fs_file *f, char *buffer, uint32_t n)
{
	struct fs_dirent *d = f->d;
	struct fs_block_map *bm = f->private_data;
	const struct fs_dirent_ops *ops = d->ops;
	uint32_t i, total_copy_length = 0;
	uint32_t direct_addresses_start = bm->block, direct_addresses_end =  bm->block + (bm->offset + n) / bm->block_size;
	uint32_t start_offset = bm->offset, end_offset = (bm->offset + n) % bm->block_size;
	uint32_t end_len = direct_addresses_end * bm->block_size + end_offset;

	if (!(FS_FILE_WRITE & f->mode))
		return -1;

	if (end_len > d->sz && (!ops->resize || ops->resize(d, end_len) < 0))
		return -1;

	for (i = direct_addresses_start; i <= direct_addresses_end; i++) {
		char buffer_part[bm->block_size];
		char *copy_start = buffer_part;
		uint32_t buffer_part_len = bm->block_size;
		memset(buffer_part, 0, sizeof(buffer_part));
		if (i == direct_addresses_start) {
			copy_start += start_offset;
			buffer_part_len -= start_offset;
		}
		if (i == direct_addresses_end)
			buffer_part_len -= bm->block_size - end_offset;
		if (!ops->read_block || ops->read_block(d, buffer_part, i) < 0)
			return -1;
		memcpy(copy_start, buffer + total_copy_length, buffer_part_len);
		if (!ops->write_block || ops->write_block(d, buffer_part, i) < 0)
			return -1;
		total_copy_length += buffer_part_len;
	}

	bm->block = direct_addresses_end;
	bm->offset = end_offset;

	return total_copy_length;
}


int fs_mkfs(struct fs *f, uint32_t device_no)
{
	if (f->mkfs)
	{
		return f->mkfs(device_no);
	}
	return 0;
}
