#include "kevinfs_ata.h"
#include "kevinfs.h"

#include "../ata.h"
#include "../hashtable.h"
#include "../string.h"
#define RESERVED_BIT_TABLE_LEN 1031
#define CONTAINERS(total, container_size) \
	(total / container_size + (total % container_size == 0 ? 0 : 1))

static struct hash_set *reserved_bits;
static struct kevinfs_superblock super;
static bool is_init = 0;

static int init_reserved_hashset()
{
	reserved_bits = hash_set_init(RESERVED_BIT_TABLE_LEN);
	return reserved_bits ? 0 : -1;
}

static int read_superblock()
{
	struct kevinfs_superblock check;
	uint8_t buffer[FS_BLOCKSIZE];
	if (kevinfs_ata_read_block(0, buffer) < 0)
		return -1;
	memcpy(&check, buffer, sizeof(struct kevinfs_superblock));
	if (check.magic == FS_MAGIC) {
		memcpy(&super, &check, sizeof(struct kevinfs_superblock));
		return 0;
	}
	return -1;
}

static int create_superblock()
{
	uint8_t wbuffer[FS_BLOCKSIZE];
	int num_blocks;
	int ata_blocksize;
	uint32_t superblock_num_blocks,  available_blocks, free_blocks,
		 total_inodes, total_bits, inode_sector_size,
		 inode_bit_sector_size, data_bit_sector_size;
	if (!ata_probe(0, &num_blocks, &ata_blocksize, 0) || num_blocks == 0)
		return -1;

	superblock_num_blocks = CONTAINERS(sizeof(struct kevinfs_superblock), FS_BLOCKSIZE);
	available_blocks = num_blocks - superblock_num_blocks;
	free_blocks = (uint32_t) ((double) (available_blocks) /
			(1.0 +
			 (double) (sizeof(struct kevinfs_inode) + .125)/(4.0 * FS_BLOCKSIZE) +
			 .125/(FS_BLOCKSIZE)
			 )
			);
	total_inodes = free_blocks / 8;
	total_bits = free_blocks;
	inode_sector_size = CONTAINERS((total_inodes * sizeof(struct kevinfs_inode)), FS_BLOCKSIZE);
	inode_bit_sector_size = CONTAINERS(total_bits, FS_BLOCKSIZE);
	data_bit_sector_size = CONTAINERS(total_bits, FS_BLOCKSIZE);

	super.magic = FS_MAGIC;
	super.blocksize = FS_BLOCKSIZE;
	super.physical_blocksize = ata_blocksize;
	super.inode_bitmap_start = superblock_num_blocks;
	super.inode_start = super.inode_bitmap_start + inode_bit_sector_size;
	super.block_bitmap_start = super.inode_start + inode_sector_size;
	super.free_block_start = super.block_bitmap_start + data_bit_sector_size;
	super.num_inodes = total_inodes;
	super.num_free_blocks = free_blocks;

	memcpy(wbuffer, &super, sizeof(super));
	return kevinfs_ata_write_block(0, wbuffer);
}

static int get_available_bit(uint32_t index, uint32_t *res)
{
	uint32_t bit_index;
	uint8_t bit_buffer[FS_BLOCKSIZE];
	if (kevinfs_ata_read_block(index, bit_buffer) < 0)
		return -1;

	for (bit_index = 0; bit_index < sizeof(bit_buffer); bit_index++) {
		if (bit_buffer[bit_index] != 255) {
			uint8_t bit = (1u << 7);
			uint32_t offset;
			for (offset = 0; offset < sizeof(uint8_t) * 8; offset += 1) {
				uint32_t potential_result;
				if (!(bit_buffer[bit_index] & bit)) {
					potential_result = index * FS_BLOCKSIZE + bit_index * sizeof(uint8_t) * 8 + offset;
					if(hash_set_add(reserved_bits, potential_result) == 0) {
						*res = potential_result;
						return 0;
					}
				}
				bit >>= 1;
			}
		}
	}
	return -1;
}

int kevinfs_ata_read_block(uint32_t index, void *buffer)
{
	uint32_t num_blocks = FS_BLOCKSIZE/ATA_BLOCKSIZE;
	int ret = ata_read(0, buffer, num_blocks, index);
	return ret;
}

int kevinfs_ata_write_block(uint32_t index, void *buffer)
{
	uint32_t num_blocks = FS_BLOCKSIZE/ATA_BLOCKSIZE;
	int ret = ata_write(0, buffer, num_blocks, index);
	return ret;
}

int kevinfs_ata_set_bit(uint32_t index, uint32_t begin, uint32_t end)
{
	uint8_t bit_buffer[FS_BLOCKSIZE];
	uint32_t bit_block_index = index / (8 * FS_BLOCKSIZE);
	uint32_t bit_block_offset = index % (8 * FS_BLOCKSIZE);
	uint8_t bit_mask = 1u << (7 - bit_block_offset % 8);
        uint32_t key_for_hash = begin * FS_BLOCKSIZE + index;

	hash_set_delete(reserved_bits, key_for_hash);
	if (kevinfs_ata_read_block(begin + bit_block_index, bit_buffer) < 0)
		return -1;
	if ((bit_mask & bit_buffer[bit_block_offset / 8]) > 0)
		return -1;

	bit_buffer[bit_block_offset / 8] |= bit_mask;
	return kevinfs_ata_write_block(begin + bit_block_index, bit_buffer);
}

int kevinfs_ata_unset_bit(uint32_t index, uint32_t begin, uint32_t end)
{
	uint8_t bit_buffer[FS_BLOCKSIZE];
	uint32_t bit_block_index = index / (8 * FS_BLOCKSIZE);
	uint32_t bit_block_offset = index % (8 * FS_BLOCKSIZE);
	uint8_t bit_mask = 1u << (7 - bit_block_offset % 8);

	if (kevinfs_ata_read_block(begin + bit_block_index, bit_buffer) < 0)
		return -1;
	if ((bit_mask & bit_buffer[bit_block_offset / 8]) == 0)
		return -1;
	bit_buffer[bit_block_offset / 8] ^= bit_mask;
	return kevinfs_ata_write_block(begin + bit_block_index, bit_buffer);
}

int kevinfs_ata_check_bit(uint32_t index, uint32_t begin, uint32_t end, bool *res)
{
	uint8_t bit_buffer[FS_BLOCKSIZE];
	uint32_t bit_block_index = index / (8 * FS_BLOCKSIZE);
	uint32_t bit_block_offset = index % (8 * FS_BLOCKSIZE);
	uint8_t bit_mask = 1u << (7 - bit_block_offset % 8);
        uint32_t key_for_hash = begin * FS_BLOCKSIZE + index;

	*res = hash_set_lookup(reserved_bits, key_for_hash);
	if (*res) {
		return 0;
	}
	if (kevinfs_ata_read_block(begin + bit_block_index, bit_buffer) < 0) {
		return -1;
	}
	*res = (bit_mask & bit_buffer[bit_block_offset / 8]) != 0;
	return 0;
}

int kevinfs_ata_fkevinfs_range(uint32_t start, uint32_t end, uint32_t *res)
{
	uint32_t index;
	int result;

	for (index = start; index < end; index++) {
		result = get_available_bit(index, res);
		if (result == 0) {
			*res -= start * FS_BLOCKSIZE;
			return 0;
		}
	}
	return -1;
}

int kevinfs_ata_init(bool *already_formatted)
{
	*already_formatted = 0;
	if (!read_superblock())
		*already_formatted = 1;
	else if (create_superblock() < 0)
		return -1;

	return init_reserved_hashset();
}

struct kevinfs_superblock *kevinfs_ata_get_superblock()
{
	if (is_init)
		return &super;
	if (!read_superblock())
		return &super;
	return 0;
}
