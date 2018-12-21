#ifndef FS_H
#define FS_H

#define FS_FILE_READ (1 << 0)
#define FS_FILE_WRITE (1 << 1)

#include "kernel/types.h"
#include "device.h"

struct fs;
struct fs_volume;
struct fs_dirent;
struct fs_file;

struct fs_dirent *fs_resolve(const char *path);

struct fs *fs_lookup(const char *name);

int fs_volume_format(struct fs *f, struct device *d);
struct fs_volume *fs_volume_open(struct fs *f, struct device *d );
struct fs_volume *fs_volume_addref(struct fs_volume *v);
struct fs_dirent *fs_volume_root(struct fs_volume *);
int fs_volume_close(struct fs_volume *v);

struct fs_dirent *fs_dirent_namei(struct fs_dirent *d, const char *path);
struct fs_dirent *fs_dirent_addref(struct fs_dirent *d);
int fs_dirent_readdir(struct fs_dirent *d, char *buffer, int buffer_length);
int fs_dirent_rmdir(struct fs_dirent *d, const char *name);
int fs_dirent_link(struct fs_dirent *d, const char *oldpath, const char *newpath);
int fs_dirent_unlink(struct fs_dirent *d, const char *name);
int fs_dirent_mkdir(struct fs_dirent *d, const char *name);
int fs_dirent_mkfile(struct fs_dirent *d, const char *name);
int fs_dirent_size(struct fs_dirent *d );
int fs_dirent_isdir(struct fs_dirent *d);
int fs_dirent_close(struct fs_dirent *d);

struct fs_file *fs_file_open(struct fs_dirent *d, uint8_t mode);
struct fs_file *fs_file_addref(struct fs_file *f);
int fs_file_read(struct fs_file *f, char *buffer, uint32_t length, uint32_t offset);
int fs_file_write(struct fs_file *f, const char *buffer, uint32_t length, uint32_t offset);
int fs_file_size(struct fs_file *f );
int fs_file_close(struct fs_file *f);

void fs_register(struct fs *f);

#endif
