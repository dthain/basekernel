#ifndef FS_H
#define FS_H

#include "kerneltypes.h"
#include "list.h"

struct fs {
	struct list_node node;
	char *name;
	struct fs_volume *(*mount)(uint32_t device_no);
};

struct fs_file {
	void *private_data;
	struct fs_dirent *d;
	uint32_t sz;
};

struct fs_volume {
	void *private_data;
	const struct fs_volume_ops *ops;
	uint32_t block_size;
};

struct fs_dirent {
	void *private_data;
	struct fs_volume *v;
	uint32_t sz;
	const struct fs_dirent_ops *ops;
};

struct fs *fs_get(const char *name);
int fs_register(struct fs *f);

struct fs_volume *fs_volume_mount(struct fs *f, uint32_t device_no);
int fs_volume_umount(struct fs_volume *v);
struct fs_dirent *fs_volume_root(struct fs_volume *);

struct fs_file *fs_file_open(struct fs_dirent *d, uint8_t mode);
int fs_dirent_close(struct fs_dirent *d);
int fs_file_read(struct fs_file *f, char *buffer, uint32_t n);
int fs_file_close(struct fs_file *f);
struct fs_dirent *fs_dirent_namei(struct fs_dirent *d, const char *path);
int fs_dirent_readdir(struct fs_dirent *d, char *buffer, int buffer_length);
int fs_dirent_rmdir(struct fs_dirent *d, const char *name);
int fs_dirent_link(struct fs_dirent *d, const char *oldpath, const char *newpath);
int fs_dirent_unlink(struct fs_dirent *d, const char *name);

#endif
