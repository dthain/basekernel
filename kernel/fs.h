#ifndef FS_H
#define FS_H

#define FS_FILE_READ (1 << 0)
#define FS_FILE_WRITE (1 << 1)

#include "kernel/types.h"

struct fs {
	char *name;
	const struct fs_ops *ops;
	struct fs *next;
};

struct fs_volume {
	struct fs *fs;
	uint32_t block_size;
	int refcount;
	void *private_data;
};

struct fs_dirent {
	struct fs_volume *v;
	uint32_t size;
	int refcount;
	void *private_data;
};

struct fs_file {
	struct fs_dirent *d;
	uint32_t size;
	int8_t mode;
	int refcount;
	void *private_data;
};

struct fs_dirent *fs_resolve(const char *path);

void fs_register(struct fs *f);
struct fs *fs_lookup(const char *name);
int fs_mkfs(struct fs *f, uint32_t device_no);

struct fs_volume *fs_volume_open(struct fs *f, uint32_t device_no);
struct fs_volume *fs_volume_addref(struct fs_volume *v);
struct fs_dirent *fs_volume_root(struct fs_volume *);
int fs_volume_close(struct fs_volume *v);

struct fs_file *fs_file_open(struct fs_dirent *d, uint8_t mode);
struct fs_file *fs_file_addref(struct fs_file *f);
int fs_file_read(struct fs_file *f, char *buffer, uint32_t length, uint32_t offset);
int fs_file_write(struct fs_file *f, const char *buffer, uint32_t length, uint32_t offset);
int fs_file_close(struct fs_file *f);

struct fs_dirent *fs_dirent_namei(struct fs_dirent *d, const char *path);
struct fs_dirent *fs_dirent_addref(struct fs_dirent *d);
int fs_dirent_readdir(struct fs_dirent *d, char *buffer, int buffer_length);
int fs_dirent_rmdir(struct fs_dirent *d, const char *name);
int fs_dirent_link(struct fs_dirent *d, const char *oldpath, const char *newpath);
int fs_dirent_unlink(struct fs_dirent *d, const char *name);
int fs_dirent_mkdir(struct fs_dirent *d, const char *name);
int fs_dirent_mkfile(struct fs_dirent *d, const char *name);
int fs_dirent_compare(struct fs_dirent *d1, struct fs_dirent *d2, int *result);
int fs_dirent_close(struct fs_dirent *d);

int fs_dirent_dup(struct fs_dirent* src, struct fs_dirent* dst);

struct fs_ops {
	struct fs_dirent *(*root) (struct fs_volume * d);
	struct fs_volume *(*mount) (int unit);
	int (*umount) (struct fs_volume * d);
	int (*mkfs) (int unit);
	int (*close) (struct fs_dirent * d);
	int (*mkdir) (struct fs_dirent * d, const char *name);
	int (*mkfile) (struct fs_dirent * d, const char *name);

	struct fs_dirent *(*lookup) (struct fs_dirent * d, const char *name);
	int (*readdir) (struct fs_dirent * d, char *buffer, int buffer_length);
	int (*rmdir) (struct fs_dirent * d, const char *name);
	int (*link) (struct fs_dirent * d, const char *oldpath, const char *newpath);
	int (*unlink) (struct fs_dirent * d, const char *name);
	int (*read_block) (struct fs_dirent * d, char *buffer, uint32_t blocknum);
	int (*write_block) (struct fs_dirent * d, const char *buffer, uint32_t blocknum);
	int (*resize) (struct fs_dirent * d, uint32_t blocks);
	int (*compare) (struct fs_dirent * d1, struct fs_dirent * d2, int *result);
};

#endif
