#ifndef FS_OPS_H
#define FS_OPS_H

struct fs_volume_ops {
	struct fs_dirent *(*root)(struct fs_volume *d);
	int (*umount)(struct fs_volume *d);
};

struct fs_dirent_ops {
	struct fs_file *(*open)(struct fs_dirent *d, int8_t mode);
	int (*close)(struct fs_dirent *d);
	struct fs_dirent *(*mkdir)(struct fs_dirent *d, const char *name);
	struct fs_dirent *(*lookup)(struct fs_dirent *d, const char *name);
	int (*readdir)(struct fs_dirent *d, char *buffer, int buffer_length);
	int (*rmdir)(struct fs_dirent *d, const char *name);
	int (*link)(struct fs_dirent *d, const char *oldpath, const char *newpath);
	int (*unlink)(struct fs_dirent *d, const char *name);
	int (*read_block)(struct fs_dirent *d, char *buffer, uint32_t blocknum);
	int (*write_block)(struct fs_dirent *d, char *buffer, uint32_t blocknum);
};

#endif
