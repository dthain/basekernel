/*
Copyright (C) 2015-2019 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/

#ifndef X86_H
#define X86_H

#include "kernel/types.h"

#define X86_SEGMENT_SELECTOR( seg, rpl )  (((seg)<<3)+(rpl))

#define X86_SEGMENT_KERNEL_CODE  X86_SEGMENT_SELECTOR(1,0)
#define X86_SEGMENT_KERNEL_DATA  X86_SEGMENT_SELECTOR(2,0)
#define X86_SEGMENT_USER_CODE    X86_SEGMENT_SELECTOR(3,3)
#define X86_SEGMENT_USER_DATA    X86_SEGMENT_SELECTOR(4,3)
#define X86_SEGMENT_TSS          X86_SEGMENT_SELECTOR(5,0)

struct x86_eflags {
	unsigned carry:1;
	unsigned reserved0:1;
	unsigned parity:1;
	unsigned reserved1:1;

	unsigned auxcarry:1;
	unsigned reserved2:1;
	unsigned zero:1;
	unsigned sign:1;

	unsigned trap:1;
	unsigned interrupt:1;
	unsigned direction:1;
	unsigned overflow:1;

	unsigned iopl:2;
	unsigned nested:1;
	unsigned reserved3:1;

	unsigned resume:1;
	unsigned v86:1;
	unsigned align:1;
	unsigned vinterrupt:1;

	unsigned vpending:1;
	unsigned id:1;
};

struct x86_regs {
	int32_t eax;
	int32_t ebx;
	int32_t ecx;
	int32_t edx;
	int32_t esi;
	int32_t edi;
	int32_t ebp;
};

struct x86_stack {
	struct x86_regs regs2;
	int32_t old_ebp;
	int32_t old_eip;
	struct x86_regs regs1;
	int32_t ds;
	int32_t intr_num;
	int32_t intr_code;
	int32_t eip;
	int32_t cs;
	struct x86_eflags eflags;
	int32_t esp;
	int32_t ss;
};

struct x86_segment {
	uint16_t limit0;
	uint16_t base0;

	uint8_t base1;

	unsigned type:4;
	unsigned stype:1;
	unsigned dpl:2;
	unsigned present:1;

	unsigned limit1:4;
	unsigned avail:1;
	unsigned zero:1;
	unsigned size:1;
	unsigned granularity:1;

	uint8_t base2;
};

struct x86_tss {
	int16_t prev;
	int16_t reserved;
	int32_t esp0;
	int16_t ss0;
	int16_t reserved0;
	int32_t esp1;
	int16_t ss1;
	int16_t reserved1;
	int32_t esp2;
	int16_t ss2;
	int16_t reserved2;
	int32_t cr3;
	int32_t eip;
	int32_t eflags;
	int32_t eax;
	int32_t ecx;
	int32_t edx;
	int32_t ebx;
	int32_t esp;
	int32_t ebp;
	int32_t esi;
	int32_t edi;
	int16_t es;
	int16_t reserved3;
	int16_t cs;
	int16_t reserved4;
	int16_t ss;
	int16_t reserved5;
	int16_t ds;
	int16_t reserved6;
	int16_t fs;
	int16_t reserved7;
	int16_t gs;
	int16_t reserved8;
	int16_t ldt;
	int16_t reserved9;
	int16_t t;
	int16_t iomap;
};

struct x86_gdt_init {
	int16_t size;
	struct x86_segment *base;
};

#endif
