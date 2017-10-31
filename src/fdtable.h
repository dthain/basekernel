#ifndef FDTABLE_H
#define FDTABLE_H
#define MAX_FD_COUNT (1u<<10)

#include "kerneltypes.h"

struct fdtable_entry {
	struct kevinfs_inode *inode;
	uint32_t offset;
	uint8_t mode;
};

struct fdtable {
	struct fdtable_entry *fd_array[MAX_FD_COUNT];
};

int fdtable_add(struct fdtable *table, struct kevinfs_inode *inode, uint8_t mode);
int fdtable_rm(struct fdtable *table, int fd);
struct fdtable_entry *fdtable_get(struct fdtable *table, int fd);

struct fdtable_entry *fdtable_entry_init(struct kevinfs_inode *node, uint8_t mode);
int fdtable_entry_seek_offset(struct fdtable_entry *entry, uint32_t nbytes, bool clamp);
int fdtable_entry_seek_absolute(struct fdtable_entry *entry, uint32_t offset);

#endif
