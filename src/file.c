#include "file.h"
#include "kmalloc.h"
#include "string.h"
#include "process.h"

#define IN_MEMORY_CHUNK_SIZE 1024
#define IN_MEMORY_INODE_COUNT 10

static memory_inode_t *in_memory_fs;
static char *in_memory_fs_data;
static int memory_inode_count;

int file_init()
{
	in_memory_fs = kmalloc(IN_MEMORY_INODE_COUNT * sizeof(memory_inode_t));
	in_memory_fs_data = kmalloc(IN_MEMORY_CHUNK_SIZE * IN_MEMORY_INODE_COUNT);
	memory_inode_count = 0;

	return 0;
}

int register_memory_inode(const char *filename)
{
	memory_inode_t *new_node = in_memory_fs + memory_inode_count;
	new_node->data_start = in_memory_fs_data +  memory_inode_count * IN_MEMORY_CHUNK_SIZE;
	new_node->filename = filename;
	memory_inode_count++;

	return 0;
}

int file_memory_inode_index(const char *name)
{
	int i;
	for (i = 0; i < memory_inode_count; i++)
	{
		if (!strcmp(name, in_memory_fs[i].filename))
			return i;
	}
	return -1;
}
