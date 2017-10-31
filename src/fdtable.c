#include "kmalloc.h"
#include "string.h"
#include "fdtable.h"
#include "kevinfs/kevinfs.h"

struct fdtable_entry *fdtable_entry_init(struct kevinfs_inode *node, uint8_t mode)
{
	struct fdtable_entry *res = kmalloc(sizeof(struct fdtable_entry));
	if (!res)
		return 0;
	memset(res, 0, sizeof(struct fdtable_entry));
	res->inode = node;
	res->mode = mode;
	return res;
}

int fdtable_add(struct fdtable *table, struct kevinfs_inode *node, uint8_t mode)
{
	uint32_t i;
	for (i = 0; i < MAX_FD_COUNT; i++) {
		if (table->fd_array[i] == 0) {
			struct fdtable_entry *entry = fdtable_entry_init(node, mode);
			if (!entry)
				return 0;
			table->fd_array[i] = entry;
			return i;
		}
	}
	return -1;
}

int fdtable_rm(struct fdtable *table, int fd)
{
	if (fd >= 0 && fd < MAX_FD_COUNT && table->fd_array[fd]) {
		kfree(table->fd_array[fd]);
		table->fd_array[fd] = 0;
		return 0;
	}
	return -1;
}

struct fdtable_entry *fdtable_get(struct fdtable *table, int fd)
{
	if (fd >= 0 && fd < MAX_FD_COUNT) {
		return table->fd_array[fd];
	}
	return 0;
}

int fdtable_entry_seek_offset(struct fdtable_entry *entry, uint32_t nbytes, bool clamp)
{
	struct kevinfs_inode *inode = entry->inode;
	uint32_t offset = entry->offset;

	if (offset + nbytes > FS_INODE_MAXBLOCKS * FS_BLOCKSIZE)
		return -1;

	if (offset + nbytes >= inode->sz && clamp) {
		entry->offset = inode->sz;
		return inode->sz - offset;
	}
	entry->offset += nbytes;

	return nbytes;
}

int fdtable_entry_seek_absolute(struct fdtable_entry *entry, uint32_t offset)
{
	if (offset > FS_INODE_MAXBLOCKS * FS_BLOCKSIZE)
		return -1;
	entry->offset = offset;
	return 0;
}
