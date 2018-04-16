#include "kerneltypes.h"
#include "device.h"

struct ata_buffer;

int ata_cache_read(struct ata_buffer *buf, int block, void *data);
int ata_cache_delete (struct ata_buffer *buf, int block);
int ata_cache_drop_lru(struct ata_buffer *buf);
int ata_cache_add(struct ata_buffer *buf, int block, void *data);
struct ata_buffer *ata_cache_init(int block_size);
