#include "kerneltypes.h"
#include "device.h"

struct buffer;

int buffer_read(struct buffer *buf, int block, void *data);
int buffer_delete (struct buffer *buf, int block);
int buffer_drop_lru(struct buffer *buf);
int buffer_add(struct buffer *buf, int block, void *data);
struct buffer *buffer_init(int block_size);
