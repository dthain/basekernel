/*
Copyright (C) 2015-2019 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/

/*
This file describes the basic memory layout of the machine,
and is included by both C and assembly code, so it can only
contain #defines.
*/

/*
Note that addresses in 16-bit mode are described as segment+offset,
where the segment must be aligned on a 16-byte boundary.
So, BOOTBLOCK_START = BOOTBLOCK_SEGMENT<<4 + BOOTBLOCK_OFFSET
*/

/*
The bootblock is loaded at the fixed address of 0x7c00,
which is a constant defined by the PC hardware.
*/

#define BOOTBLOCK_START   0x7c00
#define BOOTBLOCK_SEGMENT 0x07c0
#define BOOTBLOCK_OFFSET  0x0000

/*
We choose the initial stack to start at 0xfff0 and grow
downward.  This stack location is used in the startup code,
and by the kernel when no process is active.
*/

#define INTERRUPT_STACK_TOP     0xfff0
#define INTERRUPT_STACK_SEGMENT 0x0000
#define INTERRUPT_STACK_OFFSET  0xfff0

/*
We choose the kernel code to start at 0x10000 (64KB).
Code is loaded into this location by the bootblock.
*/

#define KERNEL_START   0x10000
#define KERNEL_SEGMENT 0x1000
#define KERNEL_OFFSET  0x0000

/*
The total size of the kernel (in bytes) is stored at this
offset, relative to the start of the kernel code.
The bootblock uses this to decide how much to load from disk.
*/

#define KERNEL_SIZE_OFFSET 20

/*
Following the kernel code is a direct mapper memory area
set aside for kmalloc() which implements a list of small
memory allocations for internal kernel purposes.
*/

#define KMALLOC_START  0x100000
#define KMALLOC_LENGTH 0x100000

/*
Main memory starts at the 2MB boundary following the kmalloc area.
This area is tracked by memory.c and used for allocatable pages, which can
be consumed by either the kernel or user processes.  The end of
main memory is determined dynamically at bootup time.
*/

#define MAIN_MEMORY_START  0x200000

/*
We choose the user-mode address space to begin at 0x80000000,
and the user-mode stack to start at the top of memory and
grow down.  Addresses below 0x80000000 are all supervisor mode.
*/

#define PROCESS_ENTRY_POINT 0x80000000
#define PROCESS_STACK_INIT  0xfffffff0
