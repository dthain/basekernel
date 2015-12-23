/*
Copyright (C) 2015 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/

#include "interrupt.h"
#include "console.h"
#include "pic.h"
#include "process.h"
#include "kernelcore.h"

static interrupt_handler_t interrupt_handler_table[48];
static uint32_t interrupt_count[48];
static uint8_t interrupt_spurious[48];

static const char * exception_names[] = {
	"division by zero",
        "debug exception",
        "nonmaskable interrupt",
        "breakpoint",
        "overflow",
        "bounds check",
        "invalid instruction",
        "coprocessor error",
        "double fault",
        "copressor overrun",
        "invalid task",
        "segment not present",
        "stack exception",
        "general protection fault",
        "page fault",
        "unknown",
        "coprocessor error"
};

static void unknown_exception( int i, int code )
{
	unsigned vaddr, paddr;

	if(i==14) {
		asm("mov %%cr2, %0" : "=r" (vaddr) );
		if(pagetable_getmap(current->pagetable,vaddr,&paddr)) {
			console_printf("interrupt: illegal page access at vaddr %x\n",vaddr);
			process_dump(current);
			process_exit(0);
		} else {
			printf("interrupt: page fault at %x\n",vaddr);
			printf("please write a fault handler in interrupt.c!\n");
			printf("kernel halted.\n");
			halt();
		}
	} else {
		console_printf("interrupt: exception %d: %s (code %x)\n",i,exception_names[i],code);
		process_dump(current);
	}

	if(current) {
		process_exit(0);
	} else {
		console_printf("interrupt: exception in kernel code!\n");
		halt();
	}
}

static void unknown_hardware( int i, int code )
{
	if(!interrupt_spurious[i]) {
		console_printf("interrupt: spurious interrupt %d\n",i);
	}
	interrupt_spurious[i]++;
}

void interrupt_register( int i, interrupt_handler_t handler )
{
	interrupt_handler_table[i] = handler;
}

static void interrupt_acknowledge( int i )
{
	if(i<32) {
		/* do nothing */
	} else {
		pic_acknowledge(i-32);
	}
}

void interrupt_init()
{
	int i;
	pic_init(32,40);
	for(i=32;i<48;i++) {
		interrupt_disable(i);
		interrupt_acknowledge(i);
	}
	for(i=0;i<32;i++) {
		interrupt_handler_table[i] = unknown_exception;
		interrupt_spurious[i] = 0;
		interrupt_count[i] = 0;
	}
	for(i=32;i<48;i++) {
		interrupt_handler_table[i] = unknown_hardware;
		interrupt_spurious[i] = 0;
		interrupt_count[i] = 0;
	}

	interrupt_unblock();

	console_printf("interrupt: ready\n");
}

void interrupt_handler( int i, int code )
{
	(interrupt_handler_table[i]) (i,code);
	interrupt_acknowledge(i);
	interrupt_count[i]++;
}

void interrupt_enable( int i )
{
	if(i<32) {
		/* do nothing */
	} else {
		pic_enable(i-32);
	}
}

void interrupt_disable( int i )
{
	if(i<32) {
		/* do nothing */
	} else {
		pic_disable(i-32);
	}
}

void interrupt_block()
{
	asm("cli");
}

void interrupt_unblock()
{
	asm("sti");
}

void interrupt_wait()
{
	asm("sti");
	asm("hlt");
}

