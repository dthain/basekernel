/*
Copyright (C) 2015 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/

#include "process.h"
#include "fs.h"
#include "console.h"
#include "kobject.h"
#include "memory.h"
#include "string.h"
#include "list.h"
#include "x86.h"
#include "interrupt.h"
#include "memorylayout.h"
#include "kmalloc.h"
#include "kernel/types.h"
#include "kernelcore.h"
#include "main.h"
#include "keyboard.h"
#include "clock.h"

struct process *current = 0;
struct list ready_list = { 0, 0 };
struct list grave_list = { 0, 0 };
struct list grave_watcher_list = { 0, 0 }; // parent processes are put here to wait for their children
struct process *process_table[PROCESS_MAX_PID] = { 0 };

void process_init()
{
	current = process_create();

	pagetable_load(current->pagetable);
	pagetable_enable();

	//set up initial kobject descriptors
	current->ktable[0] = kobject_create_device(keyboard_get());
	current->ktable[1] = kobject_create_device(console_get());
	current->ktable[2] = current->ktable[1];
	current->ktable[3] = kobject_create_graphics(&graphics_root);
	graphics_root.count++;

	current->state = PROCESS_STATE_READY;

	current->waiting_for_child_pid = 0;

	console_printf("process: ready\n");
}

void process_kstack_reset(struct process *p, unsigned entry_point)
{
	struct x86_stack *s;

	p->state = PROCESS_STATE_CRADLE;

	s = (struct x86_stack *) p->kstack_ptr;

	s->regs2.ebp = (uint32_t) (p->kstack_ptr + 28);
	s->old_ebp = (uint32_t) (p->kstack_ptr + 32);
	s->old_eip = (unsigned) intr_return;
	s->ds = X86_SEGMENT_USER_DATA;
	s->cs = X86_SEGMENT_USER_CODE;
	s->eip = entry_point;
	s->eflags.interrupt = 1;
	s->eflags.iopl = 3;
	s->esp = PROCESS_STACK_INIT;
	s->ss = X86_SEGMENT_USER_DATA;
}

void process_kstack_copy(struct process *parent, struct process *child)
{
	child->kstack_top = child->kstack + PAGE_SIZE - 8;
	child->kstack_ptr = child->kstack_top - sizeof(struct x86_stack);

	struct x86_stack *child_regs = (struct x86_stack *) child->kstack_ptr;
	struct x86_stack *parent_regs = (struct x86_stack *) (parent->kstack_top - sizeof(struct x86_stack));

	*child_regs = *parent_regs;

	child_regs->regs2.ebp = (uint32_t) (child->kstack_ptr + 28);
	child_regs->old_ebp = (uint32_t) (child->kstack_ptr + 32);
	child_regs->old_eip = (unsigned) intr_return;
	child_regs->regs1.eax = 0;
}

/*
Valid pids start at 1 and go to PROCESS_MAX_PID.
To avoid confusion, keep picking increasing
pids until it is necessary to wrap around.
"last" is the most recently selected pid.
*/

static int process_allocate_pid()
{
	static int last = 0;

	int i;

	for(i = last + 1; i < PROCESS_MAX_PID; i++) {
		if(!process_table[i]) {
			last = i;
			return i;
		}
	}

	for(i = 1; i < last; i++) {
		if(!process_table[i]) {
			last = i;
			return i;
		}
	}

	return 0;
}

void process_inherit(struct process *parent, struct process *child)
{
	/* Copy kernel objects */
	memcpy(child->ktable, parent->ktable, sizeof(parent->ktable));
	int i;
	for(i = 0; i < PROCESS_MAX_OBJECTS; i++) {
		if(child->ktable[i]) {
			child->ktable[i] = kobject_addref(parent->ktable[i]);
		}
	}

	/* Inherit current and root directories */
	child->current_dir = fs_dirent_addref(parent->current_dir);
	child->root_dir = fs_dirent_addref(parent->root_dir);

	/* Set the parent of the new process to the calling process */
	child->ppid = parent->pid;
}

int process_data_size_set(struct process *p, unsigned size)
{
	// XXX check valid ranges
	// XXX round up to page size

	if(size % PAGE_SIZE) {
		size += (PAGE_SIZE - size % PAGE_SIZE);
	}

	if(size > p->vm_data_size) {
		uint32_t start = PROCESS_ENTRY_POINT + p->vm_data_size;
		pagetable_alloc(p->pagetable, start, size, PAGE_FLAG_USER | PAGE_FLAG_READWRITE | PAGE_FLAG_CLEAR);
	} else if(size < p->vm_data_size) {
		uint32_t start = PROCESS_ENTRY_POINT + size;
		pagetable_free(p->pagetable, start, p->vm_data_size);
	} else {
		// requested size is equal to current.
	}

	p->vm_data_size = size;
	pagetable_refresh();

	return 0;
}

int process_stack_size_set(struct process *p, unsigned size)
{
	// XXX check valid ranges
	// XXX round up to page size

	if(size > p->vm_stack_size) {
		uint32_t start = -size;
		pagetable_alloc(p->pagetable, start, size - p->vm_stack_size, PAGE_FLAG_USER | PAGE_FLAG_READWRITE | PAGE_FLAG_CLEAR);
	} else {
		uint32_t start = -p->vm_stack_size;
		pagetable_free(p->pagetable, start, p->vm_stack_size - size);
	}

	p->vm_stack_size = size;
	pagetable_refresh();

	return 0;
}

void process_stack_reset(struct process *p, unsigned size)
{
	process_stack_size_set(p, size);
	memset((void *) -size, size, 0);
}

struct process *process_create()
{
	struct process *p;

	p = memory_alloc_page(1);

	p->pid = process_allocate_pid();
	process_table[p->pid] = p;

	p->pagetable = pagetable_create();
	pagetable_init(p->pagetable);

	p->vm_data_size = 0;
	p->vm_stack_size = 0;

	process_data_size_set(p, 2 * PAGE_SIZE);
	process_stack_size_set(p, 2 * PAGE_SIZE);

	p->kstack = memory_alloc_page(1);
	p->kstack_top = p->kstack + PAGE_SIZE - 8;
	p->kstack_ptr = p->kstack_top - sizeof(struct x86_stack);

	process_kstack_reset(p, PROCESS_ENTRY_POINT);

	// XXX table should be allocated
	int i;
	for(i = 0; i < PROCESS_MAX_OBJECTS; i++) {
		p->ktable[i] = 0;
	}

	p->state = PROCESS_STATE_READY;

	return p;
}

void process_delete(struct process *p)
{
	int i;
	for(i = 0; i < PROCESS_MAX_OBJECTS; i++) {
		if(p->ktable[i]) {
			kobject_close(p->ktable[i]);
		}
	}
	fs_dirent_close(p->current_dir);
	fs_dirent_close(p->root_dir);
	pagetable_delete(p->pagetable);
	memory_free_page(p->kstack);
	memory_free_page(p);
	process_table[p->pid] = 0;
}

void process_launch(struct process *p)
{
	list_push_tail(&ready_list, &p->node);
}

static void process_switch(int newstate)
{
	interrupt_block();

	if(current) {
		if(current->state != PROCESS_STATE_CRADLE) {
			asm("pushl %ebp");
			asm("pushl %edi");
			asm("pushl %esi");
			asm("pushl %edx");
			asm("pushl %ecx");
			asm("pushl %ebx");
			asm("pushl %eax");
		      asm("movl %%esp, %0":"=r"(current->kstack_ptr));
		}

		interrupt_stack_pointer = (void *) INTERRUPT_STACK_TOP;
		current->state = newstate;

		if(newstate == PROCESS_STATE_READY) {
			list_push_tail(&ready_list, &current->node);
		}
		if(newstate == PROCESS_STATE_GRAVE) {
			list_push_tail(&grave_list, &current->node);
		}
	}

	current = 0;

	while(1) {
		current = (struct process *) list_pop_head(&ready_list);
		if(current)
			break;

		interrupt_unblock();
		interrupt_wait();
		interrupt_block();
	}

	current->state = PROCESS_STATE_RUNNING;
	interrupt_stack_pointer = current->kstack_top;

	asm("movl %0, %%cr3"::"r"(current->pagetable));
	asm("movl %0, %%esp"::"r"(current->kstack_ptr));

	asm("popl %eax");
	asm("popl %ebx");
	asm("popl %ecx");
	asm("popl %edx");
	asm("popl %esi");
	asm("popl %edi");
	asm("popl %ebp");

	interrupt_unblock();
}

int allow_preempt = 0;

void process_preempt()
{
	if(allow_preempt && current && ready_list.head) {
		process_switch(PROCESS_STATE_READY);
	}
}

void process_yield()
{
	process_switch(PROCESS_STATE_READY);
}

void process_exit(int code)
{
	console_printf("process %d exiting with status %d...\n", current->pid, code);
	current->exitcode = code;
	current->exitreason = PROCESS_EXIT_NORMAL;
	process_wakeup_parent(&grave_watcher_list); // On exit, wake up parent if need be
	process_switch(PROCESS_STATE_GRAVE);
}

void process_wait(struct list *q)
{
	list_push_tail(q, &current->node);
	process_switch(PROCESS_STATE_BLOCKED);
}

void process_wakeup(struct list *q)
{
	struct process *p;
	p = (struct process *) list_pop_head(q);
	if(p) {
		p->state = PROCESS_STATE_READY;
		list_push_tail(&ready_list, &p->node);
	}
}

void process_reap_all()
{
	struct process *p;
	while((p = (struct process *) list_pop_head(&grave_list))) {
		process_delete(p);
	}
}

/* Wakes up parent off of the corresponding list*/
void process_wakeup_parent(struct list *q)
{
	struct process *p = (struct process *) q->head;
	// Loop through all the waiting parents to see if one needs to be woken up
	while(p) {
		if (p->pid == current->ppid && (p->waiting_for_child_pid == 0 || p->waiting_for_child_pid == current->pid)) {
			p->state = PROCESS_STATE_READY;
			p->waiting_for_child_pid = 0;
			list_remove(&p->node);
			list_push_tail(&ready_list, &p->node);
			break;
		}
		p = (struct process *) (&p->node)->next;
	}
}

void process_wakeup_all(struct list *q)
{
	struct process *p;
	while((p = (struct process *) list_pop_head(q))) {
		p->state = PROCESS_STATE_READY;
		list_push_tail(&ready_list, &p->node);
	}
}

void process_dump(struct process *p)
{
	struct x86_stack *s = (struct x86_stack *) (INTERRUPT_STACK_TOP - sizeof(*s));
	console_printf("kstack: %x\n", p->kstack);
	console_printf("stackp: %x\n", p->kstack_ptr);
	console_printf("eax: %x     cs: %x\n", s->regs1.eax, s->cs);
	console_printf("ebx: %x     ds: %x\n", s->regs1.ebx, s->ds);
	console_printf("ecx: %x     ss: %x\n", s->regs1.ecx, s->ss);
	console_printf("edx: %x eflags: %x\n", s->regs1.edx, s->eflags);
	console_printf("esi: %x\n", s->regs1.esi);
	console_printf("edi: %x\n", s->regs1.edi);
	console_printf("ebp: %x\n", s->regs1.ebp);
	console_printf("esp: %x\n", s->esp);
	console_printf("eip: %x\n", s->eip);
}

int process_available_fd(struct process *p)
{
	struct kobject **fdtable = current->ktable;
	int i;
	for(i = 0; i < PROCESS_MAX_OBJECTS; i++) {
		if(fdtable[i] == 0)
			return i;
	}
	return -1;
}

void process_make_dead(struct process *dead)
{
	int i;
	for(i = 0; i < PROCESS_MAX_PID; i++) {
		if(process_table[i] && process_table[i]->ppid == dead->pid) {
			process_make_dead(process_table[i]);
		}
	}
	dead->exitcode = 0;
	dead->exitreason = PROCESS_EXIT_KILLED;
	if(dead == current) {
		process_switch(PROCESS_STATE_GRAVE);
	} else {
		list_remove(&dead->node);
		list_push_tail(&grave_list, &dead->node);
	}
}

int process_kill(uint32_t pid)
{
	if(pid > 0 && pid <= PROCESS_MAX_PID) {
		struct process *dead = process_table[pid];
		if(dead) {
			console_printf("process killed\n");
			process_make_dead(dead);
			return 0;
		} else {
			return 1;
		}
	} else {
		return 1;
	}
}

int process_wait_child(uint32_t pid, struct process_info *info, int timeout)
{
	clock_t start, elapsed;
	uint32_t total;

	if(!info)
		return -1;

	start = clock_read();

	do {
		struct process *p = (struct process *) (grave_list.head);
		while(p) {
			struct process *next = (struct process *) p->node.next;
			if((pid != 0 && p->pid == pid) || (p->ppid == current->pid)) {
				info->exitcode = p->exitcode;
				info->exitreason = p->exitreason;
				info->pid = p->pid;
				return p->pid;
			}
			p = next;
		}

		current->waiting_for_child_pid = pid;
		process_wait(&grave_watcher_list);

		elapsed = clock_diff(start, clock_read());
		total = elapsed.millis + elapsed.seconds * 1000;
	} while(total < timeout || timeout < 0);

	return 0;
}

int process_reap(uint32_t pid)
{
	struct process *p = (struct process *) (grave_list.head);
	while(p) {
		struct process *next = (struct process *) p->node.next;
		if(p->pid == pid) {
			list_remove(&p->node);
			process_delete(p);
			return 0;
		}
		p = next;
	}
	return 1;
}

void process_pass_arguments(struct process *p, const char **argv, int argc)
{
	/* Copy command line arguments */
	struct x86_stack *s = (struct x86_stack *) p->kstack_ptr;
	unsigned paddr;
	pagetable_getmap(p->pagetable, PROCESS_STACK_INIT - PAGE_SIZE + 0x10, &paddr, 0);
	char *esp = (char *) paddr + PAGE_SIZE - 0x10;
	char *ebp = esp;
	/* Copy each argument */
	int i;
	for(i = 0; i < argc; i++) {
		ebp -= MAX_ARGV_LENGTH;
		strncpy(ebp, argv[i], MAX_ARGV_LENGTH - 1);
	}
	/* Set pointers to each argument (argv) */
	for(i = argc; i > 0; --i) {
		ebp -= 4;
		*((char **) (ebp)) = ((char *) (PROCESS_STACK_INIT - MAX_ARGV_LENGTH * i));
	}
	/* Set argumetns for _start on the stack */
	*((char **) (ebp - 12)) = (char *) (PROCESS_STACK_INIT - MAX_ARGV_LENGTH * argc - 4 * argc);
	*((int *) (ebp - 8)) = argc;
	s->esp -= (esp - ebp) + 16;
}

int process_stats(int pid, struct proc_stats *s) {
	if (pid > PROCESS_MAX_PID || !process_table[pid]) {
		return 1;
	}
 *s = process_table[pid]->stats;
 return 0;
}
