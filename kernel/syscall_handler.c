/*
Copyright (C) 2015 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/

#include "kernel/syscall.h"
#include "kernel/gfxstream.h"
#include "syscall_handler.h"
#include "console.h"
#include "keyboard.h"
#include "process.h"
#include "kmalloc.h"
#include "kobject.h"
#include "cdromfs.h"
#include "string.h"
#include "memorylayout.h"
#include "main.h"
#include "fs.h"
#include "kobject.h"
#include "pagetable.h"
#include "clock.h"
#include "rtc.h"
#include "elf.h"
#include "kmalloc.h"
#include "memory.h"
#include "ata.h"
#include "graphics.h"

// Get rid of this once we have a proper dirlist stream
#define LSDIR_TEMP_BUFFER_SIZE 250

int sys_debug(const char *str)
{
	console_printf("%s", str);
	return 0;
}

int sys_process_exit(int status)
{
	process_exit(status);
	return 0;
}

int sys_process_yield()
{
	process_yield();
	return 0;
}

int sys_sbrk(int delta)
{
	process_data_size_set(current, current->vm_data_size + delta);
	return PROCESS_ENTRY_POINT + current->vm_data_size;
}

/* Helper routines to duplicate/free an argv array locally */

static char ** argv_copy( int argc, const char **argv )
{
	char ** pp;

	pp = kmalloc(sizeof(char*)*argc);
	int i;
	for(i=0;i<argc;i++) {
		pp[i] = strdup(argv[i]);
	}

	return pp;
}

static void argv_delete( int argc, char **argv )
{
	int i;
	for(i=0;i<argc;i++) {
		kfree(argv[i]);
	}
	kfree(argv);
}

/*
process_run() creates a child process in a more efficient
way than fork/exec by creating the child without duplicating
the memory state, then loading
*/

int sys_process_run(const char *path, const char **argv, int argc )
{
	/* Copy argv and path into kernel memory. */
	char **copy_argv = argv_copy(argc,argv);
	char *copy_path = strdup(path);

	/* Create the child process */
	struct process *p = process_create();
	process_inherit(current, p);

	/* SWITCH TO ADDRESS SPACE OF CHILD PROCESS */
	struct pagetable *old_pagetable = current->pagetable;
	current->pagetable = p->pagetable;
	pagetable_load(p->pagetable);

	/* Attempt to load the program image. */
	addr_t entry;
	int r = elf_load(p, copy_path, &entry);
	if(r >= 0) {
		/* If load succeeded, reset stack and pass arguments */
		process_stack_reset(p, PAGE_SIZE);
		process_kstack_reset(p, entry);
		process_pass_arguments(p,argc,copy_argv);
	}

	/* SWITCH BACK TO ADDRESS SPACE OF PARENT PROCESS */
	current->pagetable = old_pagetable;
	pagetable_load(old_pagetable);

	/* Delete the argument and path copies. */
	argv_delete(argc,copy_argv);
	kfree(copy_path);

	/* If any error happened, return in the context of the parent */
	if(r < 0) {
		if(r == KERROR_EXECUTION_FAILED) {
			process_delete(p);
		}
		return r;
	}

	/* Otherwise, launch the new child process. */
	process_launch(p);
	return p->pid;
}

/*
	Function creates a child process with the standard window replaced by
	a window of size w and h
*/
int sys_process_wrun(const char *path, const char **argv, int argc, int * window_desc, int wd_parent )
{
	/* Copy argv array into kernel memory. */
	char **copy_argv = argv_copy(argc,argv);

	/* Open new window in parrent */
	int wd_child = sys_open_window(wd_parent, window_desc[0], window_desc[1], window_desc[2], window_desc[3]);

	/* Create the child process */
	struct process *p = process_create();
	process_inherit(current, p);

	/* SWITCH TO ADDRESS SPACE OF CHILD PROCESS */
	struct pagetable *old_pagetable = current->pagetable;
	current->pagetable = p->pagetable;
	pagetable_load(p->pagetable);

	/* Attempt to load the program image. */
	addr_t entry;
	int r = elf_load(p, path, &entry);
	if(r >= 0) {
		/* If load succeeded, reset stack and pass arguments */
		process_stack_reset(p, PAGE_SIZE);
		process_kstack_reset(p, entry);
		process_pass_arguments(p,argc,copy_argv);
	}

	/* SWITCH BACK TO ADDRESS SPACE OF PARENT PROCESS */
	current->pagetable = old_pagetable;
	pagetable_load(old_pagetable);

	/* Delete the argument copy. */
	argv_delete(argc,copy_argv);

	/* Make wd child's std window */
	if(p->ktable[wd_parent]) {
		kobject_close(p->ktable[wd_parent]);
	}
	p->ktable[wd_parent] = kobject_addref(p->ktable[wd_child]);
	
	/* 
		Close the new windows old descriptor for both the child and the parent.
		This way, one process doesnt have any duplicates and other children wont 
		accidentally inherit another siblings standard window.
	*/
	kobject_close(p->ktable[wd_child]);
	kobject_close(current->ktable[wd_child]);

	/* If any error happened, return in the context of the parent */
	if(r < 0) {
		if(r == KERROR_EXECUTION_FAILED) {
			process_delete(p);
		}
		return r;
	}

	/* Otherwise, launch the new child process. */
	process_launch(p);
	return p->pid;
}

int sys_process_exec(const char *path, const char **argv, int argc)
{
	addr_t entry;

	/* Duplicate the arguments into kernel space */
	char **copy_argv = argv_copy(argc,argv);

	/* Attempt to load the program image into this process. */
	int r = elf_load(current, path, &entry);

	/* On failure, return only if our address space is not corrupted. */
	if(r < 0) {
		if(r == KERROR_EXECUTION_FAILED) {
			process_kill(current->pid);
		}
		argv_delete(argc,copy_argv);
		return r;
	}

	/* Reset the stack and pass in the program arguments */
	process_stack_reset(current, PAGE_SIZE);
	process_kstack_reset(current, entry );
	process_pass_arguments(current, argc, copy_argv );

	/* Delete the local copy of the arguments. */
	argv_delete(argc,copy_argv);

	/*
	   IMPORTANT: Following a successful exec, we cannot return via
	   the normal path, because our stack has been reset to that
	   of a fresh process.  We must switch in order to jump
	   to the new stack properly.
	 */

	process_yield();

	/* NOTREACHED */
	return 0;
}

int sys_process_fork()
{
	struct process *p = process_create();
	p->ppid = current->pid;
	pagetable_delete(p->pagetable);
	p->pagetable = pagetable_duplicate(current->pagetable);
	process_inherit(current, p);
	process_kstack_copy(current, p);
	process_launch(p);
	return p->pid;
}

int sys_process_sleep(unsigned int ms)
{
	clock_wait(ms);
	return 0;
}

int sys_process_self()
{
	return current->pid;
}

int sys_process_parent()
{
	return current->ppid;
}

int sys_process_kill(int pid)
{
	return process_kill(pid);
}

int sys_process_wait(struct process_info *info, int timeout)
{
	return process_wait_child(0, info, timeout);
}

int sys_process_reap(int pid)
{
	return process_reap(pid);
}

uint32_t sys_gettimeofday()
{
	struct rtc_time t;
	rtc_read(&t);
	return rtc_time_to_timestamp(&t);
}

int sys_chdir(const char *path)
{
	struct fs_dirent *d = fs_resolve(path);
	if(d) {
		fs_dirent_close(current->current_dir);
		current->current_dir = d;
		return 0;
	} else {
		// XXX get error back from namei
		return KERROR_NOT_FOUND;
	}
}

int sys_readdir(const char *path, char *buffer, int len)
{
	struct fs_dirent *d = fs_resolve(path);
	if(d) {
		return fs_dirent_readdir(d, buffer, len);
	} else {
		// XXX get error back from namei
		return KERROR_NOT_FOUND;
	}
}

int sys_mkdir(const char *path)
{
	// XXX doesn't work -- separate parent and new directory.

	return fs_dirent_mkdir(current->current_dir, path);
}

int sys_rmdir(const char *path)
{
	struct fs_dirent *d = fs_resolve(path);
	if(d) {
		// XXX this API doesn't make sense.
		return fs_dirent_rmdir(d, path);
	} else {
		// XXX get error back from namei
		return -1;
	}
}

int sys_open(const char *path, int mode, int flags)
{
	int fd = process_available_fd(current);
	if(fd < 0)
		return -1;

	struct fs_dirent *d = fs_resolve(path);
	if(!d) {
		// XXX creating in current_dir, not parent dir!
		fs_dirent_mkfile(current->current_dir, path);
		// XXX return value not checked!
		d = fs_dirent_namei(current->current_dir, path);
	}
	struct fs_file *fp = fs_file_open(d, mode);
	current->ktable[fd] = kobject_create_file(fp);
	return fd;
}

int sys_object_type(int fd)
{
	int fd_type = kobject_get_type(current->ktable[fd]);
	if(!fd_type)
		return 0;
	return fd_type;
}

int sys_dup(int fd1, int fd2)
{
	if(fd1 < 0 || fd1 >= PROCESS_MAX_OBJECTS || !current->ktable[fd1] || fd2 >= PROCESS_MAX_OBJECTS) {
		return KERROR_NOT_FOUND;
	}
	if(fd2 < 0) {
		fd2 = process_available_fd(current);
		if(!fd2) {
			return KERROR_NOT_FOUND;
		}
	}
	if(current->ktable[fd2]) {
		kobject_close(current->ktable[fd2]);
	}
	current->ktable[fd2] = kobject_addref(current->ktable[fd1]);
	return fd2;
}

int sys_read(int fd, void *data, int length)
{
	struct kobject *p = current->ktable[fd];
	return kobject_read(p, data, length);
}

int sys_read_nonblock(int fd, void *data, int length)
{
	struct kobject *p = current->ktable[fd];
	return kobject_read_nonblock(p, data, length);
}


int sys_write(int fd, void *data, int length)
{
	struct kobject *p = current->ktable[fd];
	return kobject_write(p, data, length);
}

int sys_lseek(int fd, int offset, int whence)
{
	// XXX add kobject method here
	return KERROR_NOT_IMPLEMENTED;
}

int sys_close(int fd)
{
	struct kobject *p = current->ktable[fd];
	kobject_close(p);
	current->ktable[fd] = 0;
	return 0;
}

int sys_pwd(char *result)
{
	struct fs_dirent *d = current->current_dir;
	char dir_list[LSDIR_TEMP_BUFFER_SIZE];
	memset(dir_list, 0, LSDIR_TEMP_BUFFER_SIZE);
	result[0] = 0;
	while(1) {
		struct fs_dirent *parent = fs_dirent_namei(d, "..");
		int hit_root, found_child;
		fs_dirent_compare(parent, d, &hit_root);
		if(hit_root) {
			kfree(parent);
			break;
		}
		if(fs_dirent_readdir(parent, dir_list, LSDIR_TEMP_BUFFER_SIZE) < 0)
			return -1;
		char *dir = strtok(dir_list, " ");
		while(dir) {
			struct fs_dirent *child = fs_dirent_namei(parent, dir);
			fs_dirent_compare(child, d, &found_child);
			if(found_child) {
				char result_next[LSDIR_TEMP_BUFFER_SIZE];
				memset(result_next, 0, LSDIR_TEMP_BUFFER_SIZE);
				strcat(result_next, "/");
				strcat(result_next, dir);
				strcat(result_next, result);
				if(strlen(result) + strlen(result_next) + 1 > LSDIR_TEMP_BUFFER_SIZE)
					return -1;
				strcpy(result, result_next);
				break;
			}
			dir = strtok(dir + strlen(dir) + 1, " ");
		}
		d = parent;
	}
	return 0;
}

int sys_open_pipe()
{
	int fd = process_available_fd(current);
	if(fd < 0) {
		return KERROR_NOT_FOUND;
	}
	struct pipe *p = pipe_open();
	if(!p) {
		return KERROR_NOT_FOUND;
	}
	current->ktable[fd] = kobject_create_pipe(p);
	return fd;
}

int sys_set_blocking(int fd, int b)
{
	struct kobject *p = current->ktable[fd];
	return kobject_set_blocking(p, b);
}

int sys_open_console(int wd)
{
	int fd = process_available_fd(current);
	if(wd < 0 || fd < 0) {
		return KERROR_NOT_FOUND;
	}
	struct device *d = console_create(current->ktable[wd]->data.graphics);
	if(!d) {
		return KERROR_NOT_FOUND;
	}
	current->ktable[fd] = kobject_create_device(d);
	return fd;
}

// XXX don't go into kobject internals

int sys_open_window(int wd, int x, int y, int w, int h)
{
	int fd = process_available_fd(current);
	if(fd == -1 || wd < 0 || current->ktable[wd]->type != KOBJECT_GRAPHICS || current->ktable[wd]->data.graphics->clip.w < x + w || current->ktable[wd]->data.graphics->clip.h < y + h) {
		return KERROR_NOT_FOUND;
	}

	current->ktable[fd] = kobject_create_graphics(graphics_create(current->ktable[wd]->data.graphics));

	if(!current->ktable[fd]) {
		return KERROR_NOT_FOUND;
	}

	current->ktable[fd]->data.graphics->clip.x = x + current->ktable[wd]->data.graphics->clip.x;
	current->ktable[fd]->data.graphics->clip.y = y + current->ktable[wd]->data.graphics->clip.y;
	current->ktable[fd]->data.graphics->clip.w = w;
	current->ktable[fd]->data.graphics->clip.h = h;

	return fd;
}

int sys_get_dimensions(int fd, int * dims, int n) {
	struct kobject *p = current->ktable[fd];
	return kobject_get_dimensions(p, dims, n);
}


int sys_sys_stats(struct sys_stats *s) {
	struct rtc_time t = {0};
	rtc_read(&t);
	s->time = rtc_time_to_timestamp(&t) - boottime;
	struct ata_count a = ata_stats();
	for (int i = 0; i < 4; i++) {
		s->blocks_written[i] = a.blocks_written[i];
		s->blocks_read[i] = a.blocks_read[i];
	}
	return 0;
}

int sys_process_stats(struct proc_stats *s, int pid) {
	return process_stats(pid, s);
}

int32_t syscall_handler(syscall_t n, uint32_t a, uint32_t b, uint32_t c, uint32_t d, uint32_t e)
{
	if ((n < MAX_SYSCALL) && current) {
		current->stats.syscall_count[n]++;
	}
	switch (n) {
	case SYSCALL_DEBUG:
		return sys_debug((const char *) a);
	case SYSCALL_PROCESS_EXIT:
		return sys_process_exit(a);
	case SYSCALL_PROCESS_YIELD:
		return sys_process_yield();
	case SYSCALL_PROCESS_SLEEP:
		return sys_process_sleep(a);
	case SYSCALL_PROCESS_SELF:
		return sys_process_self();
	case SYSCALL_PROCESS_PARENT:
		return sys_process_parent();
	case SYSCALL_PROCESS_RUN:
		return sys_process_run((const char *) a, (const char **) b, c);
	case SYSCALL_PROCESS_WRUN:
		return sys_process_wrun((const char *) a, (const char **) b, c, (int *) d, e);
	case SYSCALL_PROCESS_FORK:
		return sys_process_fork();
	case SYSCALL_PROCESS_EXEC:
		return sys_process_exec((const char *) a, (const char **) b, c);
	case SYSCALL_PROCESS_KILL:
		return sys_process_kill(a);
	case SYSCALL_PROCESS_WAIT:
		return sys_process_wait((struct process_info *) a, b);
	case SYSCALL_PROCESS_REAP:
		return sys_process_reap(a);
	case SYSCALL_OPEN:
		return sys_open((const char *) a, b, c);
	case SYSCALL_DUP:
		return sys_dup(a, b);
	case SYSCALL_READ:
		return sys_read(a, (void *) b, c);
	case SYSCALL_READ_NONBLOCK:
		return sys_read_nonblock(a, (void *) b, c);
	case SYSCALL_WRITE:
		return sys_write(a, (void *) b, c);
	case SYSCALL_LSEEK:
		return sys_lseek(a, b, c);
	case SYSCALL_CLOSE:
		return sys_close(a);
	case SYSCALL_OBJECT_TYPE:
		return sys_object_type(a);
	case SYSCALL_SET_BLOCKING:
		return sys_set_blocking(a, b);
	case SYSCALL_OPEN_PIPE:
		return sys_open_pipe();
	case SYSCALL_OPEN_CONSOLE:
		return sys_open_console(a);
	case SYSCALL_OPEN_WINDOW:
		return sys_open_window(a, b, c, d, e);
	case SYSCALL_GET_DIMENSIONS:
		return sys_get_dimensions(a, (int *) b, c);
	case SYSCALL_GETTIMEOFDAY:
		return sys_gettimeofday();
	case SYSCALL_SBRK:
		return sys_sbrk(a);
	case SYSCALL_CHDIR:
		return sys_chdir((const char *) a);
	case SYSCALL_MKDIR:
		return sys_mkdir((const char *) a);
	case SYSCALL_READDIR:
		return sys_readdir((const char *) a, (char *) b, (int) c);
	case SYSCALL_RMDIR:
		return sys_rmdir((const char *) a);
	case SYSCALL_PWD:
		return sys_pwd((char *) a);
	case SYSCALL_SYS_STATS:
		return sys_sys_stats((struct sys_stats *) a);
	case SYSCALL_PROCESS_STATS:
		return sys_process_stats((struct proc_stats *) a, b);
	default:
		return -1;
	}
}
