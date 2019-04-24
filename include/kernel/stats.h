#ifndef KERNEL_STATS_H
#define KERNEL_STATS_H

#include "kernel/types.h"
#include "kernel/syscall.h"

#define OBJECT_STATS_LEVEL 1
#define CONSOLE_STATS_LEVEL 2
#define GRAPHICS_STATS_LEVEL 2
#define DEVICE_STATS_LEVEL 2
#define PIPE_STATS_LEVEL 2
#define FILE_STATS_LEVEL 2
#define DIR_STATS_LEVEL 2

struct system_stats {
	int time;
	int blocks_read[4];
	int blocks_written[4];
};

struct device_stats {
	int reads;
	int writes;
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

struct console_stats {
	int writes;
	uint64_t bytes_written;
};

struct file_stats {
	int reads;
	int writes;
};

struct graphics_stats {
	int writes;
};

struct object_stats {
	int reads;
	int writes;
	uint64_t bytes_read;
	uint64_t bytes_written;
};

struct pipe_stats {
	int reads;
	int writes;
};

struct process_stats {
	int blocks_read;
	int blocks_written;
	int bytes_read;
	int bytes_written;
	int syscall_count[MAX_SYSCALL];
};

#endif
