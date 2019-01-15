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

typedef long long int64_t;
typedef int int32_t;
typedef short int16_t;
typedef char int8_t;

typedef unsigned long long uint64_t;
typedef unsigned int uint32_t;
typedef unsigned short uint16_t;
typedef unsigned char uint8_t;

typedef uint8_t bool;

typedef uint32_t addr_t;

struct rtc_time {
	uint8_t second;
	uint8_t minute;
	uint8_t hour;
	uint8_t day;
	uint8_t month;
	uint16_t year;
};

struct process_info {
	int pid;
	int exitcode;
	int exitreason;
};

typedef enum {
	KOBJECT_FILE,
	KOBJECT_DIR,
	KOBJECT_DEVICE,
	KOBJECT_GRAPHICS,
	KOBJECT_PIPE,
	KOBJECT_CONSOLE
} kobject_type_t;

typedef enum {
	KERNEL_FLAGS_READ=0,
	KERNEL_FLAGS_WRITE=1,
	KERNEL_FLAGS_CREATE=2,
	KERNEL_FLAGS_RANDOM=4,
	KERNEL_FLAGS_DIRECT=8
} kernel_flags_t;


#endif
