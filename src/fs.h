#ifndef FS_H
#define FS_H

#include "kerneltypes.h"

struct fs {
	char *name;
	struct volume *(*mount)(uint32_t device_no);
};

struct file {
	void *private_data;
	uint32_t sz;
	int (*read)(struct file *f, char *buffer, uint32_t n);
	int (*write)(struct file *f, char *buffer, uint32_t n);
	int (*close)(struct file *f);
};

struct volume {
	void *private_data;
	struct dirent *(*root)(struct volume *d);
	int (*umount)(struct volume *d);
};

struct dirent {
	void *private_data;
	struct volume *v;
	uint32_t sz;

	struct file *(*open)(struct dirent *d, int8_t mode);
	int (*close)(struct dirent *d);
	struct dirent *(*mkdir)(struct dirent *d, const char *name);
	struct dirent *(*lookup)(struct dirent *d, const char *name);
	int (*readdir)(struct dirent *d, char *buffer, int buffer_length);
	int (*rmdir)(struct dirent *d, const char *name);
	int (*link)(struct dirent *d, const char *oldpath, const char *newpath);
	int (*unlink)(struct dirent *d, const char *name);
};

struct fs *fs_get(const char *name);
int fs_register(struct fs *f);

struct volume *fs_mount(struct fs *f, uint32_t device_no);
int fs_umount(struct volume *v);
struct dirent *fs_root(struct volume *);

struct file *fs_open(struct dirent *d, uint8_t mode);
int fs_read(struct file *f, char *buffer, uint32_t n);
int fs_dirent_close(struct dirent *d);
int fs_close(struct file *f);
struct dirent *fs_create(struct dirent *d, const char *name);
struct dirent *fs_lookup(struct dirent *d, const char *name);
int fs_readdir(struct dirent *d, char *buffer, int buffer_length);
int fs_rmdir(struct dirent *d, const char *name);
int fs_link(struct dirent *d, const char *oldpath, const char *newpath);
int fs_unlink(struct dirent *d, const char *name);

#endif
