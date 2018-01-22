#ifndef FS_ATA_H
#define FS_ATA_H

#include "../kerneltypes.h"
#include "kevinfs.h"

int kevinfs_ata_read_block(int device_no, uint32_t index, void *block);
int kevinfs_ata_write_block(int device_no, uint32_t index, void *block);
int kevinfs_ata_set_bit(int device_no, uint32_t index, uint32_t start, uint32_t end);
int kevinfs_ata_unset_bit(int device_no, uint32_t index, uint32_t start, uint32_t end);
int kevinfs_ata_check_bit(int device_no, uint32_t index, uint32_t start, uint32_t end, bool *res);
int kevinfs_ata_ffs_range(int device_no, uint32_t start, uint32_t end, uint32_t *res);
struct kevinfs_superblock *kevinfs_ata_read_superblock(int device_no);
int kevinfs_ata_format(int device_no);
int kevinfs_ata_init();

#endif
