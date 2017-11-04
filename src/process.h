/*
Copyright (C) 2015 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/

#ifndef PROCESS_H
#define PROCESS_H

#include "kerneltypes.h"
#include "list.h"
#include "pagetable.h"
#include "x86.h"

#define PROCESS_STATE_CRADLE  0
#define PROCESS_STATE_READY   1
#define PROCESS_STATE_RUNNING 2
#define PROCESS_STATE_BLOCKED 3
#define PROCESS_STATE_GRAVE   4
#define PROCESS_MAX_WINDOWS   5

struct process {
	struct list_node node;
	int state;
	int exitcode;
	struct pagetable *pagetable;
	char *kstack;
	char *kstack_top;
	char *stack_ptr;
    struct graphics* windows[PROCESS_MAX_WINDOWS];
    int window_count;
	uint32_t entry;
	uint32_t pid;
	uint32_t ppid;
};

void process_init();

struct process * process_create( unsigned code_size, unsigned stack_size );
void process_launch( struct process *p );

void process_yield();
void process_preempt();
void process_exit( int code );
void process_dump( struct process *p );

void process_wait( struct list *q );
void process_wakeup( struct list *q );
void process_wakeup_all( struct list *q );

uint32_t process_getpid();
uint32_t process_getppid();

extern struct process *current;

#endif
