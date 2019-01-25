/*
Copyright (C) 2015-2019 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/

#ifndef PROCESS_H
#define PROCESS_H

#include "kernel/types.h"
#include "kernel/stats.h"
#include "list.h"
#include "pagetable.h"
#include "kobject.h"
#include "x86.h"
#include "fs.h"

#define PROCESS_STATE_CRADLE  0
#define PROCESS_STATE_READY   1
#define PROCESS_STATE_RUNNING 2
#define PROCESS_STATE_BLOCKED 3
#define PROCESS_STATE_GRAVE   4

#define PROCESS_MAX_OBJECTS 32
#define PROCESS_MAX_PID 1024

#define PROCESS_EXIT_NORMAL   0
#define PROCESS_EXIT_KILLED   1

struct process {
	struct list_node node;
	int state;
	int exitcode;
	int exitreason;
	struct pagetable *pagetable;
	char *kstack;
	char *kstack_top;
	char *kstack_ptr;
	struct kobject *ktable[PROCESS_MAX_OBJECTS];
	struct fs_dirent *current_dir;
	struct fs_dirent *root_dir;
	struct process_stats stats;
	uint32_t pid;
	uint32_t ppid;
	uint32_t vm_data_size;
	uint32_t vm_stack_size;
	uint32_t waiting_for_child_pid;
};

void process_init();

struct process *process_create();
void process_delete(struct process *p);
void process_launch(struct process *p);
void process_pass_arguments(struct process *p, int argc, char **argv);
void process_inherit(struct process *parent, struct process *child);
void process_selective_inherit(struct process *parent, struct process *child, int * fds, int fd_len);

void process_stack_reset(struct process *p, unsigned size);
void process_kstack_reset(struct process *p, unsigned entry_point);
void process_kstack_copy(struct process *parent, struct process *child);

int process_data_size_set(struct process *p, unsigned size);
int process_stack_size_set(struct process *p, unsigned size);

int process_available_fd(struct process *p);
int process_object_max(struct process *p);

void process_yield();
void process_preempt();
void process_exit(int code);
void process_dump(struct process *p);

void process_wait(struct list *q);
void process_wakeup(struct list *q);
void process_wakeup_parent(struct list *q);
void process_wakeup_all(struct list *q);
void process_reap_all();

int process_kill(uint32_t pid);
int process_wait_child(uint32_t pid, struct process_info *info, int timeout);
int process_reap(uint32_t pid);

int process_stats(int pid, struct process_stats *stat);

extern struct process *current;

#endif
