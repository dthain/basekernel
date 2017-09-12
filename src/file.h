#ifndef FILE_H
#define FILE_H

#include "kerneltypes.h"

#define FILE_MODE_READ (1u << 0)
#define FILE_MODE_WRITE (1u << 1)

enum inode_type
{
	DISK,
	MEMORY,
};

struct memory_inode
{
	const char *filename;
	char *data_start;
};
typedef struct memory_inode memory_inode_t;

struct file
{
	uint8_t mode;
	uint32_t offset;
	uint32_t inode;
	enum inode_type type;

	int (*open)(struct file *, char *, uint8_t);
	int (*close)(struct file *, int);
	int (*read)(struct file *, char *, uint32_t);
	int (*write)(struct file *, char *, uint32_t);
	int (*seek)(struct file *, char *, uint32_t);
};
typedef struct file file_t;

int file_init();
int register_memory_inode(const char *filename);
int file_memory_inode_index(const char *filename);

#endif
