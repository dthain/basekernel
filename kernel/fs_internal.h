/*
Copyright (C) 2016-2019 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/

#ifndef FS_INTERNAL
#define FS_INTERNAL

#include "fs.h"
#include "cdromfs.h"
#include "diskfs.h"

struct fs {
	char *name;
	const struct fs_ops *ops;
	struct fs *next;
};

struct fs_volume {
	struct fs *fs;
	struct device *device;
	uint32_t block_size;
	int refcount;
	union {
		struct cdrom_volume cdrom;
		struct diskfs_superblock disk;
	};
};

struct fs_dirent {
	struct fs_volume *volume;
	uint32_t size;
	int inumber;
	int refcount;
	int isdir;
	union {
		struct cdrom_dirent cdrom;
		struct diskfs_inode disk;
	};
};

struct fs_ops {
	struct fs_dirent *(*volume_root) (struct fs_volume *v);
	struct fs_volume *(*volume_open) (struct device *d);
	int (*volume_close) (struct fs_volume *d);
	int (*volume_format) (struct device *d);

	struct fs_dirent * (*lookup) (struct fs_dirent *d, const char *name);
	struct fs_dirent * (*mkdir) (struct fs_dirent *d, const char *name);
	struct fs_dirent * (*mkfile) (struct fs_dirent *d, const char *name);

	int (*read_block) (struct fs_dirent *d, char *buffer, uint32_t blocknum);
	int (*write_block) (struct fs_dirent *d, const char *buffer, uint32_t blocknum);
	int (*list) (struct fs_dirent *d, char *buffer, int buffer_length);
	int (*remove) (struct fs_dirent *d, const char *name);
	int (*resize) (struct fs_dirent *d, uint32_t blocks);
	int (*close) (struct fs_dirent *d);
};

#endif
