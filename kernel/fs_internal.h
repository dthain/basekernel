#ifndef FS_INTERNAL
#define FS_INTERNAL

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
	int isdir;
	void *private_data;
};

struct fs_file {
	struct fs_dirent *d;
	uint32_t size;
	int8_t mode;
	int refcount;
	void *private_data;
};

struct fs_ops {
	struct fs_dirent *(*volume_root) (struct fs_volume * d);
	struct fs_volume *(*volume_open) (struct device *d);
	int (*volume_close) (struct fs_volume * d);
	int (*volume_format) (struct device *d);

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
};

#endif
