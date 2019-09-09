#ifndef KERNEL_STATS_H
#define KERNEL_STATS_H

#include "kernel/types.h"
#include "kernel/syscall.h"

struct kernel_io_stats {
	unsigned time; // temporary
	unsigned syscall_count;
	unsigned read_ops, write_ops;
	unsigned read_bytes, write_bytes;
};

struct kernel_bcache_stats {
	int read_hits;
	int read_misses;
	int write_hits;
	int write_misses;
	int writebacks;
};

#endif
