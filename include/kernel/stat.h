#ifndef KERNEL_STAT_H
#define KERNEL_STAT_H

#include "kernel/types.h"
#include "kernel/syscall.h"

struct sys_stat {
	uint32_t time;
	uint32_t blocks_read[4];
	uint32_t blocks_written[4];
};

struct proc_stat {
	uint32_t blocks_read;
	uint32_t blocks_written;
	uint32_t bytes_read;
	uint32_t bytes_written;
	uint32_t syscall_count[MAX_SYSCALL];
};

#endif
