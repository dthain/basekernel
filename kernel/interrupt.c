/*
Copyright (C) 2015-2019 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/

#include "interrupt.h"
#include "console.h"
#include "pic.h"
#include "process.h"
#include "kernelcore.h"
#include "x86.h"

static interrupt_handler_t interrupt_handler_table[48];
static uint32_t interrupt_count[48];
static uint8_t interrupt_spurious[48];

static const char *exception_names[] = {
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

static void unknown_exception(int i, int code)
{
	unsigned vaddr; // virtual address trying to be accessed
	unsigned paddr; // physical address
	unsigned esp; // stack pointer

	if(i==14) {
		asm("mov %%cr2, %0" : "=r" (vaddr) ); // virtual address trying to be accessed		
		esp  = ((struct x86_stack *)(current->kstack_top - sizeof(struct x86_stack)))->esp; // stack pointer of the process that raised the exception
		// Check if the requested memory is in the stack or data
		int data_access = vaddr < current->vm_data_size;

		// Subtract 128 from esp because of the red-zone 
		// According to https:gcc.gnu.org, the red zone is a 128-byte area beyond 
		// the stack pointer that will not be modified by signal or interrupt handlers 
		// and therefore can be used for temporary data without adjusting the stack pointer.
		int stack_access = vaddr >= esp - 128; 

		// Check if the requested memory is already in use
		int page_already_present = pagetable_getmap(current->pagetable,vaddr,&paddr,0);
		
		// Check if page is already mapped (which will result from violating the permissions on page) or that
		// we are accessing neither the stack nor the heap, or we are accessing both. If so, error
		if (page_already_present || !(data_access ^ stack_access)) {
			printf("interrupt: illegal page access at vaddr %x\n",vaddr);
			process_dump(current);
			process_exit(0);
		} else {
			// XXX update process->vm_stack_size when growing the stack.
			pagetable_alloc(current->pagetable, vaddr, PAGE_SIZE, PAGE_FLAG_USER | PAGE_FLAG_READWRITE | PAGE_FLAG_CLEAR);
			return;
		}
	} else {
		printf("interrupt: exception %d: %s (code %x)\n", i, exception_names[i], code);
		process_dump(current);
	}

	if(current) {
		process_exit(0);
	} else {
		printf("interrupt: exception in kernel code!\n");
		halt();
	}
}

static void unknown_hardware(int i, int code)
{
	if(!interrupt_spurious[i]) {
		printf("interrupt: spurious interrupt %d\n", i);
	}
	interrupt_spurious[i]++;
}

void interrupt_register(int i, interrupt_handler_t handler)
{
	interrupt_handler_table[i] = handler;
}

static void interrupt_acknowledge(int i)
{
	if(i < 32) {
		/* do nothing */
	} else {
		pic_acknowledge(i - 32);
	}
}

void interrupt_init()
{
	int i;
	pic_init(32, 40);
	for(i = 32; i < 48; i++) {
		interrupt_disable(i);
		interrupt_acknowledge(i);
	}
	for(i = 0; i < 32; i++) {
		interrupt_handler_table[i] = unknown_exception;
		interrupt_spurious[i] = 0;
		interrupt_count[i] = 0;
	}
	for(i = 32; i < 48; i++) {
		interrupt_handler_table[i] = unknown_hardware;
		interrupt_spurious[i] = 0;
		interrupt_count[i] = 0;
	}

	interrupt_unblock();

	printf("interrupt: ready\n");
}

void interrupt_handler(int i, int code)
{
	(interrupt_handler_table[i]) (i, code);
	interrupt_acknowledge(i);
	interrupt_count[i]++;
}

void interrupt_enable(int i)
{
	if(i < 32) {
		/* do nothing */
	} else {
		pic_enable(i - 32);
	}
}

void interrupt_disable(int i)
{
	if(i < 32) {
		/* do nothing */
	} else {
		pic_disable(i - 32);
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
