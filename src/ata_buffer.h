#include "kerneltypes.h"

int ata_buffer_read(uint32_t device_no, uint32_t block_no, void *buffer);
int ata_buffer_write(uint32_t device_no, uint32_t block_no, void *buffer);
