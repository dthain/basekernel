#ifndef KERNEL_STATS_H
#define KERNEL_STATS_H

#include "kernel/types.h"
#include "kernel/syscall.h"

struct system_stats {
	uint32_t time;
	uint32_t blocks_read[4];
	uint32_t blocks_written[4];
};

struct object_stats {
	uint32_t reads;
	uint32_t writes;
	uint64_t bytes_read;
	uint64_t bytes_written;
};

struct process_stats {
	uint32_t blocks_read;
	uint32_t blocks_written;
	uint32_t bytes_read;
	uint32_t bytes_written;
	uint32_t syscall_count[MAX_SYSCALL];
};

#endif
