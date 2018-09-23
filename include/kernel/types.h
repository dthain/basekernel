#ifndef KERNELTYPES_H
#define KERNELTYPES_H

#pragma pack(2)

#define PAGE_SIZE 4096
#define PAGE_BITS 12
#define PAGE_MASK 0xfffff000

#define KILO 1024
#define MEGA (KILO*KILO)
#define GIGA (KILO*KILO*KILO)

#define MAX_ARGV_LENGTH 256

#define MIN(x,y) ( ((x)<(y)) ? (x) : (y) )
#define MAX(x,y) ( ((x)>(y)) ? (x) : (y) )

typedef int int32_t;
typedef short int16_t;
typedef char int8_t;

typedef unsigned int uint32_t;
typedef unsigned short uint16_t;
typedef unsigned char uint8_t;

typedef uint8_t bool;

typedef uint32_t addr_t;

struct sys_stat {
	uint32_t time;
	uint32_t blocks_read[4];
	uint32_t blocks_written[4];
};
struct proc_stat {
	uint32_t blocks_read[4];
	uint32_t blocks_written[4];
	uint32_t syscall_count[40]; //XXX this should be dynamic based on how many system calls there are
};

struct process_info {
	int pid;
	int exitcode;
	int exitreason;
	struct proc_stat stat;
};

#endif
