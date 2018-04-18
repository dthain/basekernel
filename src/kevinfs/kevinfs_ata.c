#include "kevinfs_ata.h"
#include "kevinfs.h"
#include "../device.h"
#include "../kmalloc.h"

#include "../hashtable.h"
#include "../string.h"
#include "../ata.h"
#define RESERVED_BIT_TABLE_LEN 1031
#define CONTAINERS(total, container_size) \
	(total / container_size + (total % container_size == 0 ? 0 : 1))

struct kevinfs_superblock *kevinfs_ata_read_superblock(struct device *device)
{
	struct kevinfs_superblock *check = kmalloc(sizeof(struct kevinfs_superblock));
	uint8_t buffer[FS_BLOCKSIZE];
	if (kevinfs_ata_read_block(device, 0, buffer) < 0)
		return 0;
	memcpy(check, buffer, sizeof(struct kevinfs_superblock));
	if (check->magic == FS_MAGIC)
		return check;
	kfree(check);
	return 0;
}

static int kevinfs_ata_write_superblock(struct device *device)
{
	uint8_t wbuffer[FS_BLOCKSIZE];
	int num_blocks;
	int ata_blocksize;
	uint32_t superblock_num_blocks,  available_blocks, free_blocks,
		 total_inodes, total_bits, inode_sector_size,
		 inode_bit_sector_size, data_bit_sector_size;
	if (!ata_probe(device->unit, &num_blocks, &ata_blocksize, 0) || num_blocks == 0)
		return -1;

	char zeros[ata_blocksize];
	memset(zeros,0,ata_blocksize);
	struct kevinfs_superblock super;
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

	uint32_t i;
	for (i = super.inode_bitmap_start; i < super.free_block_start; i++) {
		if(!device_write(device, zeros, 1, i))
			return -1;
	}
	return kevinfs_ata_write_block(device, 0, wbuffer);
}

int kevinfs_ata_format(struct device *device)
{
	return kevinfs_ata_write_superblock(device);
}


static int kevinfs_ata_get_available_bit(struct device *device, uint32_t index, uint32_t *res)
{
	uint32_t bit_index;
	uint8_t bit_buffer[FS_BLOCKSIZE];
	if (kevinfs_ata_read_block(device, index, bit_buffer) < 0)
		return -1;

	for (bit_index = 0; bit_index < sizeof(bit_buffer); bit_index++) {
		if (bit_buffer[bit_index] != 255) {
			uint8_t bit = (1u << 7);
			uint32_t offset;
			for (offset = 0; offset < sizeof(uint8_t) * 8; offset += 1) {
				if (!(bit_buffer[bit_index] & bit)) {
					*res = index * FS_BLOCKSIZE + bit_index * sizeof(uint8_t) * 8 + offset;
					return 0;
				}
				bit >>= 1;
			}
		}
	}
	return -1;
}

int kevinfs_ata_read_block(struct device *device, uint32_t index, void *buffer)
{
	uint32_t num_blocks = FS_BLOCKSIZE/ATA_BLOCKSIZE;
	return device_read(device, buffer, num_blocks, index) ? 0 : -1;
}

int kevinfs_ata_write_block(struct device *device, uint32_t index, void *buffer)
{
	uint32_t num_blocks = FS_BLOCKSIZE/ATA_BLOCKSIZE;
	return device_write(device, buffer, num_blocks, index) ? 0 : -1;
}

int kevinfs_ata_set_bit(struct device *device, uint32_t index, uint32_t begin, uint32_t end)
{
	uint8_t bit_buffer[FS_BLOCKSIZE];
	uint32_t bit_block_index = index / (8 * FS_BLOCKSIZE);
	uint32_t bit_block_offset = index % (8 * FS_BLOCKSIZE);
	uint8_t bit_mask = 1u << (7 - bit_block_offset % 8);

	if (kevinfs_ata_read_block(device, begin + bit_block_index, bit_buffer) < 0)
		return -1;
	if ((bit_mask & bit_buffer[bit_block_offset / 8]) > 0)
		return -1;

	bit_buffer[bit_block_offset / 8] |= bit_mask;
	return kevinfs_ata_write_block(device, begin + bit_block_index, bit_buffer);
}

int kevinfs_ata_unset_bit(struct device *device, uint32_t index, uint32_t begin, uint32_t end)
{
	uint8_t bit_buffer[FS_BLOCKSIZE];
	uint32_t bit_block_index = index / (8 * FS_BLOCKSIZE);
	uint32_t bit_block_offset = index % (8 * FS_BLOCKSIZE);
	uint8_t bit_mask = 1u << (7 - bit_block_offset % 8);

	if (kevinfs_ata_read_block(device, begin + bit_block_index, bit_buffer) < 0)
		return -1;
	if ((bit_mask & bit_buffer[bit_block_offset / 8]) == 0)
		return -1;
	bit_buffer[bit_block_offset / 8] ^= bit_mask;
	return kevinfs_ata_write_block(device, begin + bit_block_index, bit_buffer);
}

int kevinfs_ata_check_bit(struct device *device, uint32_t index, uint32_t begin, uint32_t end, bool *res)
{
	uint8_t bit_buffer[FS_BLOCKSIZE];
	uint32_t bit_block_index = index / (8 * FS_BLOCKSIZE);
	uint32_t bit_block_offset = index % (8 * FS_BLOCKSIZE);
	uint8_t bit_mask = 1u << (7 - bit_block_offset % 8);

	if (kevinfs_ata_read_block(device, begin + bit_block_index, bit_buffer) < 0) {
		return -1;
	}
	*res = (bit_mask & bit_buffer[bit_block_offset / 8]) != 0;
	return 0;
}

int kevinfs_ata_ffs_range(struct device *device, uint32_t start, uint32_t end, uint32_t *res)
{
	uint32_t index;
	int result;

	for (index = start; index < end; index++) {
		result = kevinfs_ata_get_available_bit(device, index, res);
		if (result == 0) {
			*res -= start * FS_BLOCKSIZE;
			return 0;
		}
	}
	return -1;
}
