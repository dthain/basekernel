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

struct memory_inode_t
{
	const char *filename;
	char *data_start;
};

struct file_t
{
	uint8_t mode;
	uint32_t offset;
	uint32_t inode;
	enum inode_type type;

	int (*read)(struct file_t *, char *, uint32_t);
	int (*write)(struct file_t *, char *, uint32_t);
};

int file_init();
int register_memory_inode(const char *filename);
int file_memory_inode_index(const char *filename);

#endif
