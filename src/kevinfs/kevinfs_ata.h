#ifndef FS_ATA_H
#define FS_ATA_H

#include "../kerneltypes.h"
#include "kevinfs.h"

int kevinfs_ata_read_block(uint32_t index, void *block);
int kevinfs_ata_write_block(uint32_t index, void *block);
int kevinfs_ata_set_bit(uint32_t index, uint32_t start, uint32_t end);
int kevinfs_ata_unset_bit(uint32_t index, uint32_t start, uint32_t end);
int kevinfs_ata_check_bit(uint32_t index, uint32_t start, uint32_t end, bool *res);
int kevinfs_ata_fkevinfs_range(uint32_t start, uint32_t end, uint32_t *res);
struct kevinfs_superblock *kevinfs_ata_get_superblock();
int kevinfs_ata_init(bool *already_formatted);

#endif
