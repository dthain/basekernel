/*
Copyright (C) 2016-2019 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/

#ifndef DISKFS_H
#define DISKFS_H

#include "kernel/types.h"

#define DISKFS_MAGIC 0xabcd4321
#define DISKFS_BLOCK_SIZE 4096
#define DISKFS_DIRECT_POINTERS 6
#define DISKFS_INODES_PER_BLOCK (DISKFS_BLOCK_SIZE/sizeof(struct diskfs_inode))
#define DISKFS_ITEMS_PER_BLOCK (DISKFS_BLOCK_SIZE/sizeof(struct diskfs_item))
#define DISKFS_POINTERS_PER_BLOCK (DISKFS_BLOCK_SIZE/sizeof(uint32_t))

struct diskfs_superblock {
	uint32_t magic;
	uint32_t block_size;
	uint32_t inode_start;
	uint32_t inode_blocks;
	uint32_t bitmap_start;
	uint32_t bitmap_blocks;
	uint32_t data_start;
	uint32_t data_blocks;
};

struct diskfs_inode {
	uint32_t inuse; // reserve for broader use.
	uint32_t size;
	uint32_t direct[DISKFS_DIRECT_POINTERS];
	uint32_t indirect;
};

#define DISKFS_ITEM_BLANK 0
#define DISKFS_ITEM_FILE 1
#define DISKFS_ITEM_DIR 2

#pragma pack(1)
struct diskfs_item {
	uint32_t inumber;
	uint8_t  type;
	uint8_t  name_length;
	char     name[26];
};
#pragma pack()

struct diskfs_block {
	union {
		struct diskfs_superblock superblock;
		struct diskfs_inode inodes[DISKFS_INODES_PER_BLOCK];
		struct diskfs_item items[DISKFS_ITEMS_PER_BLOCK];
		uint32_t pointers[DISKFS_POINTERS_PER_BLOCK];
		char     data[DISKFS_BLOCK_SIZE];
	};
};

int diskfs_init(void);

#endif
