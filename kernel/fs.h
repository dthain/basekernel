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

/*
fs_resolve is the most common interface to the filesystem code.
Given a path, it interprets it in the context of the current
process and returns a dirent.
*/

struct fs_dirent *fs_resolve(const char *path);

/*
fs_lookup returns the filesystem driver corresponding to
the given name, such as "cdromfs" or "diskfs"
*/

struct fs *fs_lookup(const char *name);

/*
A volume is an instance of a filesystem stored on a block device.
To begin using a filesystem, open the volume and retrieve the
root directory entry with fs_volume_root.
*/

int fs_volume_format(struct fs *f, struct device *d);
struct fs_volume *fs_volume_open(struct fs *f, struct device *d );
struct fs_volume *fs_volume_addref(struct fs_volume *v);
struct fs_dirent *fs_volume_root(struct fs_volume *vOB);
int fs_volume_close(struct fs_volume *v);

/*
A fs_dirent represents one directory entry (file, dir, symlink, etc)
in the filesystem tree.  It contains the basic information about
the object (size, type, etc) but not the actual contents.
*/

struct fs_dirent *fs_dirent_namei(struct fs_dirent *d, const char *path);
struct fs_dirent *fs_dirent_mkdir(struct fs_dirent *d, const char *name);
struct fs_dirent *fs_dirent_mkfile(struct fs_dirent *d, const char *name);
struct fs_dirent *fs_dirent_addref(struct fs_dirent *d);
int fs_dirent_readdir(struct fs_dirent *d, char *buffer, int buffer_length);
int fs_dirent_remove(struct fs_dirent *d, const char *name);
int fs_dirent_size(struct fs_dirent *d );
int fs_dirent_isdir(struct fs_dirent *d);
int fs_dirent_close(struct fs_dirent *d);

/*
An fs_file represents an actual file that can be read and possible
written.  To access a file, navigate to the appropriate dirent,
and then call fs_file_open on the dirent.
*/

struct fs_file *fs_file_open(struct fs_dirent *d, uint8_t mode);
struct fs_file *fs_file_addref(struct fs_file *f);
int fs_file_read(struct fs_file *f, char *buffer, uint32_t length, uint32_t offset);
int fs_file_write(struct fs_file *f, const char *buffer, uint32_t length, uint32_t offset);
int fs_file_size(struct fs_file *f );
int fs_file_close(struct fs_file *f);

/*
Register a new filesystem type, typically at system startup.
*/

void fs_register(struct fs *f);

#endif
