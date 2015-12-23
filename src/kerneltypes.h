#ifndef KERNELTYPES_H
#define KERNELTYPES_H

#pragma pack(2)

#define PAGE_SIZE 4096
#define PAGE_BITS 12
#define PAGE_MASK 0xfffff000

#define KILO 1024
#define MEGA (KILO*KILO)
#define GIGA (KILO*KILO*KILO)

typedef int   int32_t;
typedef short int16_t;
typedef char  int8_t;

typedef unsigned int   uint32_t;
typedef unsigned short uint16_t;
typedef unsigned char  uint8_t;

typedef uint8_t bool;

static inline uint16_t ntoh16( uint16_t x )
{
	return (x&0xff)<<8 | (x>>8);
}

static inline uint16_t hton16( uint16_t x )
{
	return (x&0xff)<<8 | (x>>8);
}

#endif
