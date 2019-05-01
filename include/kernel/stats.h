#ifndef KERNEL_STATS_H
#define KERNEL_STATS_H

#include "kernel/types.h"
#include "kernel/syscall.h"

typedef enum {
	OBJECT_TYPE,
	PIPE_TYPE,
	FILE_TYPE,
	DIR_TYPE,
	CONSOLE_TYPE,
	GRAPHICS_TYPE,
	DEVICE_TYPE
} object_type_t;

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

struct dirent_stats {
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
