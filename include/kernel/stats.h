#ifndef KERNEL_STATS_H
#define KERNEL_STATS_H

#include "kernel/types.h"
#include "kernel/syscall.h"

struct system_stats {
	int time;
	int blocks_read[4];
	int blocks_written[4];
};

struct device_driver_stats {
	int blocks_written;
	int blocks_read;
};

struct bcache_stats {
	int read_hits;
	int read_misses;
	int write_hits;
	int write_misses;
	int writebacks;
};

struct process_stats {
	int blocks_read;
	int blocks_written;
	int bytes_read;
	int bytes_written;
	int syscall_count[MAX_SYSCALL];
};

#endif
