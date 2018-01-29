/*
Copyright (C) 2015 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/

#include "process.h"
#include "fs.h"
#include "console.h"
#include "memory.h"
#include "string.h"
#include "list.h"
#include "x86.h"
#include "interrupt.h"
#include "memorylayout.h"
#include "kmalloc.h"
#include "kerneltypes.h"
#include "kernelcore.h"
#include "main.h"
#include "clock.h"

struct process *current=0;
struct list ready_list = {0,0};
struct list grave_list = {0,0};
struct process *processes[MAX_PID] = {0};

struct mount {
	struct list_node node;
	char *name;
	struct fs_volume *v;
};

void process_init()
{
	current = process_create(0,0);

	pagetable_load(current->pagetable);
	pagetable_enable();

    current->windows[0] = &graphics_root;
    current->window_count = 1;
    graphics_root.count++;

	current->state = PROCESS_STATE_READY;

	console_printf("process: ready\n");
}

static void process_stack_init( struct process *p )
{
	struct x86_stack *s;

	p->state = PROCESS_STATE_CRADLE;

	p->kstack_top = p->kstack+PAGE_SIZE-sizeof(*s);
	p->stack_ptr = p->kstack_top;

	s = (struct x86_stack *) p->stack_ptr;

	s->regs2.ebp = (uint32_t) (p->stack_ptr + 28);
	s->old_ebp = (uint32_t) (p->stack_ptr + 32);
	s->old_addr = (unsigned) intr_return;
	s->ds = X86_SEGMENT_USER_DATA;
	s->cs = X86_SEGMENT_USER_CODE;
	s->eip = p->entry;
	s->eflags.interrupt = 1;
	s->eflags.iopl = 3;
	s->esp = PROCESS_STACK_INIT;
	s->ss = X86_SEGMENT_USER_DATA;
}

static int process_allocate_pid() {
    static int last = 0;
    int i;
    for (i = 0; i < MAX_PID; i++) {
        int pid = 1 + ((last + i) % MAX_PID);
        if (!processes[pid-1]) {
            last = pid;
            return pid;
        }
    }
    return 0;
}

struct process * process_create( unsigned code_size, unsigned stack_size )
{
	struct process *p;

	p = memory_alloc_page(1);

	p->pagetable = pagetable_create();
	pagetable_init(p->pagetable);
	pagetable_alloc(p->pagetable,PROCESS_ENTRY_POINT,code_size,PAGE_FLAG_USER|PAGE_FLAG_READWRITE);
	pagetable_alloc(p->pagetable,PROCESS_STACK_INIT+0xF-stack_size+1,stack_size,PAGE_FLAG_USER|PAGE_FLAG_READWRITE);

	p->kstack = memory_alloc_page(1);
	p->entry = PROCESS_ENTRY_POINT;
    p->window_count = 0;
	p->pid = process_allocate_pid();
    if (p->pid) {
        processes[p->pid-1] = p;
    } else {
        return 0;
    }

	process_stack_init(p);

	return p;
}

void process_delete( struct process *p )
{
    int i;
    for (i = 0; i < p->window_count; i++) {
        if (!(--(p->windows[i]->count))) {
            kfree(p->windows[i]);
        }
    }
    pagetable_delete(p->pagetable);
    processes[p->pid-1] = 0;
	memory_free_page(p->kstack);
	memory_free_page(p);
}

void process_launch( struct process *p )
{
	list_push_tail(&ready_list,&p->node);
}

static void process_switch( int newstate )
{
	interrupt_block();

	if(current) {
		if(current->state!=PROCESS_STATE_CRADLE) {
			asm("pushl %ebp");
			asm("pushl %edi");
			asm("pushl %esi");
			asm("pushl %edx");
			asm("pushl %ecx");
			asm("pushl %ebx");
			asm("pushl %eax");
			asm("movl %%esp, %0" : "=r" (current->stack_ptr));
		}
		interrupt_stack_pointer = (void*)INTERRUPT_STACK_TOP;
		current->state = newstate;
		if(newstate==PROCESS_STATE_READY) {
			list_push_tail(&ready_list,&current->node);
		}
		if(newstate==PROCESS_STATE_GRAVE) {
			list_push_tail(&grave_list,&current->node);
		}
	}

	current = 0;

	while(1) {
		current = (struct process *) list_pop_head(&ready_list);
		if(current) break;
		interrupt_unblock();
		interrupt_wait();
		interrupt_block();
	}

	current->state = PROCESS_STATE_RUNNING;
	interrupt_stack_pointer = current->kstack_top;
	asm("movl %0, %%cr3" :: "r" (current->pagetable));
	asm("movl %0, %%esp" :: "r" (current->stack_ptr));

	asm("popl %eax");
	asm("popl %ebx");
	asm("popl %ecx");
	asm("popl %edx");
	asm("popl %esi");
	asm("popl %edi");
	asm("popl %ebp");

	interrupt_unblock();
}

int allow_preempt=0;

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

void process_exit( int code )
{
	console_printf("process exiting with status %d...\n",code);
	current->exitcode = code;
    current->exitreason = PROCESS_EXIT_NORMAL;
	process_switch(PROCESS_STATE_GRAVE);
}

void process_wait( struct list * q )
{
	list_push_tail(q,&current->node);
	process_switch(PROCESS_STATE_BLOCKED);
}

void process_wakeup( struct list *q )
{
	struct process *p;
	p = (struct process *)list_pop_head(q);
	if(p) {
		p->state = PROCESS_STATE_READY;
		list_push_tail(&ready_list,&p->node);
	}
}

void process_reap_all()
{
	struct process *p;
	while((p = (struct process*)list_pop_head(&grave_list))) {
        process_delete(p);
	}
}

void process_wakeup_all( struct list *q )
{
	struct process *p;
	while((p = (struct process*)list_pop_head(q))) {
		p->state = PROCESS_STATE_READY;
		list_push_tail(&ready_list,&p->node);
	}
}

void process_dump( struct process *p)
{
	struct x86_stack *s = (struct x86_stack*)(p->kstack+PAGE_SIZE-sizeof(*s));
	console_printf("kstack: %x\n",p->kstack);
	console_printf("stackp: %x\n",p->stack_ptr);
	console_printf("eax: %x     cs: %x\n",s->regs1.eax,s->cs);
	console_printf("ebx: %x     ds: %x\n",s->regs1.ebx,s->ds);
	console_printf("ecx: %x     ss: %x\n",s->regs1.ecx,s->ss);
	console_printf("edx: %x eflags: %x\n",s->regs1.edx,s->eflags);
	console_printf("esi: %x\n",s->regs1.esi);
	console_printf("edi: %x\n",s->regs1.edi);
	console_printf("ebp: %x\n",s->regs1.ebp);
	console_printf("esp: %x\n",s->esp);
	console_printf("eip: %x\n",s->eip);
}

uint32_t process_getpid() {
    return current->pid;
}

uint32_t process_getppid() {
    return current->ppid;
}

int process_available_fd()
{
	struct fs_file **fdtable = current->fdtable;
	for (int i = 0; i < PROCESS_MAX_FILES; i++)
	{
		if (fdtable[i] == 0)
			return i;
	}
	return -1;
}

static int process_register_mount(struct mount *m) {
	struct mount *m_final = kmalloc(sizeof(struct mount));
	memcpy(m_final, m, sizeof(struct mount));
	list_push_tail(&current->mounts, &m_final->node);
	return 0;
}

struct mount *process_mount_get(const char *name) {
	struct list_node *iter = current->mounts.head;
	while (iter) {
		struct mount *m = (struct mount *) iter;
		if (!strcmp(name, m->name)) {
			struct mount *ret = kmalloc(sizeof(struct mount));
			memcpy(ret, m, sizeof(struct mount));
			return ret;
		}
		iter = iter->next;
	}
	return 0;
}

int process_mount_as(struct fs_volume *v, const char *ns)
{
	struct mount *m = kmalloc(sizeof(struct mount));
	m->name = kmalloc(strlen(ns) + 1);
	strcpy(m->name, ns);
	m->v = v;
	process_register_mount(m);
	return 0;
}

static int process_chdir_with_cwd(const char *path)
{
	struct fs_dirent *d;
	if (!(d = fs_dirent_namei(current->cwd, path)))
		return -1;
	current->cwd = d;
	return 0;
}

int process_chdir(const char *ns, const char *path)
{
	if (!ns && !current->cwd)
		return -1;
	if (ns) {
		struct mount *m = process_mount_get(ns);
		current->cwd = fs_volume_root(m->v);
	}
	return process_chdir_with_cwd(path);
}

void process_make_dead( struct process *dead ) {
    int i;
    for (i = 0; i < MAX_PID; i++) {
        if (processes[i] && processes[i]->ppid == dead->pid) {
            process_make_dead(processes[i]);
        }
    }
    dead->exitcode = 0;
    dead->exitreason = PROCESS_EXIT_KILLED;
    if (dead == current) {
        process_switch(PROCESS_STATE_GRAVE);
    } else {
        list_remove(&dead->node);
        list_push_tail(&grave_list,&dead->node);
    }
}

int process_kill( uint32_t pid ) {
    if (pid > 0 && pid <= MAX_PID) {
        struct process *dead = processes[pid-1];
        if (dead) {
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

int process_wait_child(struct process_info *info, int timeout) {
	clock_t start, elapsed;
	uint32_t total;
	start = clock_read();
	do {
        struct process *p = (struct process*)(grave_list.head);
        while (p) {
            struct process* next = (struct process*)p->node.next;
            if (p->ppid == current->pid) {
                info->exitcode = p->exitcode;
                info->exitreason = p->exitreason;
                info->pid = p->pid;
                return 0;
            }
            p = next;
        }
		process_wait(&ready_list);
		elapsed = clock_diff(start,clock_read());
		total = elapsed.millis + elapsed.seconds*1000;
	} while(total<timeout);
    return 1;
}

int process_reap( uint32_t pid ) {
    struct process *p = (struct process*)(grave_list.head);
    while (p) {
        struct process* next = (struct process*)p->node.next;
        if (p->pid == pid) {
            list_remove(&p->node);
            process_delete(p);
            return 0;
        }
        p = next;
    }
    return 1;
}

void process_pass_arguments(struct process* p, const char** argv, int argc) {
    /* Copy command line arguments */
	struct x86_stack *s = (struct x86_stack *) p->stack_ptr;
    unsigned paddr;
    pagetable_getmap(p->pagetable,PROCESS_STACK_INIT-PAGE_SIZE+0x10,&paddr);
    char* esp = (char*)paddr+PAGE_SIZE-0x10;
    char* ebp = esp;
    /* Copy each argument */
    int i;
    for (i = 0; i < argc; i++) {
        ebp -= MAX_ARGV_LENGTH;
        strncpy(ebp, argv[i], MAX_ARGV_LENGTH-1);
    }
    /* Set pointers to each argument (argv) */
    for (i = argc; i > 0; --i) {
        ebp -= 4;
        *((char**)(ebp)) = ((char*)(PROCESS_STACK_INIT - MAX_ARGV_LENGTH*i));
    }
    /* Set argumetns for _start on the stack */
    *((char**)(ebp-12)) = (char*)(PROCESS_STACK_INIT-MAX_ARGV_LENGTH*argc-4*argc);
    *((int*)(ebp-8)) = argc;
	s->esp -= (esp-ebp)+16;
}
