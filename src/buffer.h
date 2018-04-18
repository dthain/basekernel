#include "kerneltypes.h"
#include "device.h"

struct buffer;

int ata_cache_read(struct buffer *buf, int block, void *data);
int ata_cache_delete (struct buffer *buf, int block);
int ata_cache_drop_lru(struct buffer *buf);
int ata_cache_add(struct buffer *buf, int block, void *data);
struct buffer *ata_cache_init(int block_size);
