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

typedef int int32_t;
typedef short int16_t;
typedef char int8_t;

typedef unsigned int uint32_t;
typedef unsigned short uint16_t;
typedef unsigned char uint8_t;

typedef uint8_t bool;

typedef uint32_t ptrint_t;

struct process_info {
	int pid;
	int exitcode;
	int exitreason;
};

#endif
