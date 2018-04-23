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
#include "kobject.h"
#include "x86.h"
#include "fs_space.h"
#include "fs.h"

#define PROCESS_STATE_CRADLE  0
#define PROCESS_STATE_READY   1
#define PROCESS_STATE_RUNNING 2
#define PROCESS_STATE_BLOCKED 3
#define PROCESS_STATE_GRAVE   4
#define PROCESS_STATE_FORK_CHILD   5
#define PROCESS_STATE_FORK_PARENT  6
#define PROCESS_MAX_WINDOWS   5
#define PROCESS_MAX_OBJECTS   100
#define PROCESS_MAX_FS_SPACES 5


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
    struct fs_space_ref fs_spaces[PROCESS_MAX_FS_SPACES];
    int fs_space_count;
    int cws;
	struct fs_dirent *cwd;
  int cwd_depth;
	struct list mounts;
	uint32_t entry;
	uint32_t pid;
	uint32_t ppid;
    void* brk;
};

struct process_pointer {
	struct list_node node;
    struct process *p;
};

void process_init();

struct process * process_create( unsigned code_size, unsigned stack_size, int pid);
void process_delete( struct process *p );
void process_launch( struct process *p );
void process_pass_arguments(struct process* p, const char** argv, int argc);
void process_inherit( struct process * p );

void process_fork_freeze();
void process_yield();
void process_preempt();
void process_exit( int code );
void process_dump( struct process *p );

void process_wait( struct list *q );
void process_wakeup( struct list *q );
void process_wakeup_all( struct list *q );
void process_reap_all();

int process_kill( uint32_t pid );
int process_wait_child(struct process_info* info, int timeout);
int process_reap( uint32_t pid );

uint32_t process_getpid();
uint32_t process_getppid();

int process_available_fd(struct process *p);
int process_mount_as(struct process *p, struct fs_volume *v, const char *ns);
int process_unmount(struct process *p, const char *ns);
int process_chdir(struct process *p, const char *path);

extern struct process *current;

#endif
