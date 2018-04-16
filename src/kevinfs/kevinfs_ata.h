#ifndef FS_ATA_H
#define FS_ATA_H

#include "../device.h"
#include "../kerneltypes.h"
#include "kevinfs.h"

int kevinfs_ata_read_block(struct device *device, uint32_t index, void *block);
int kevinfs_ata_write_block(struct device *device, uint32_t index, void *block);
int kevinfs_ata_set_bit(struct device *device, uint32_t index, uint32_t start, uint32_t end);
int kevinfs_ata_unset_bit(struct device *device, uint32_t index, uint32_t start, uint32_t end);
int kevinfs_ata_check_bit(struct device *device, uint32_t index, uint32_t start, uint32_t end, bool *res);
int kevinfs_ata_ffs_range(struct device *device, uint32_t start, uint32_t end, uint32_t *res);
struct kevinfs_superblock *kevinfs_ata_read_superblock(struct device *device);
int kevinfs_ata_format(struct device *device);
int kevinfs_ata_init();

#endif
