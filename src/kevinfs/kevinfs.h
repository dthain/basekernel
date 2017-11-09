/*
Copyright (C) 2017 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/

#ifndef KEVINFS_H
#define KEVINFS_H

#define FS_FILENAME_MAXLEN 255
#define FS_MAGIC 0x1209
#define FS_BLOCKSIZE 512
#define FS_SIZE (1u<<20)
#define FS_INODE_MAXBLOCKS 10
#define FS_RESERVED_BITS_COUNT 1031
#define FS_EMPTY_DIR_SIZE 2
#define FS_EMPTY_DIR_SIZE_BYTES FS_EMPTY_DIR_SIZE * sizeof(struct kevinfs_dir_record)
#define FILE_MODE_READ (1u << 0)
#define FILE_MODE_WRITE (1u << 1)

#include "../kerneltypes.h"

struct kevinfs_superblock {
	uint32_t magic;

	uint32_t blocksize;
	uint32_t physical_blocksize;

	uint32_t inode_bitmap_start;
	uint32_t inode_start;
	uint32_t block_bitmap_start;
	uint32_t free_block_start;

	uint32_t num_inodes;
	uint32_t num_free_blocks;
};

struct kevinfs_inode {
	uint32_t inode_number;

	uint32_t is_directory;
	uint32_t sz;
	uint32_t link_count;

	uint32_t direct_addresses[FS_INODE_MAXBLOCKS];
	uint32_t direct_addresses_len;
};

struct kevinfs_dir_record {
	char filename[FS_FILENAME_MAXLEN];
	uint32_t inode_number;
	bool is_directory;
	int32_t offset_to_next;
};

struct kevinfs_dir_record_list {
	struct kevinfs_dir_record *list;
	struct hash_set *changed;
	uint32_t list_len;
};

struct kevinfs_stat {
	uint32_t inode_number;
	uint8_t links;
	bool is_directory;
	uint32_t size;
	uint32_t num_blocks;
};

int kevinfs_init (void);
int kevinfs_lseek(int fd, uint32_t offset);
int kevinfs_stat(char *filename, struct kevinfs_stat *stat);

#endif
