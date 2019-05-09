/*
Copyright (C) 2015-2019 The University of Notre Dame
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
#include "page.h"
#include "ata.h"
#include "graphics.h"
#include "is_valid.h"
#include "bcache.h"

/*
syscall_handler() is responsible for decoding system calls
as they arrive, converting raw integers into the appropriate
types, depending on the system call number.  Then, each
individual handler routine checks the validity of each
argument (fd in range, valid path, etc) and invokes the
underlying system within the kernel.  Ideally, each of these
handler functions should be short (only a few lines)
and simply make use of functionality within other kernel modules.

sys_run/fork/exec are notable exceptions and could benefit
from simplification.
*/

/*
Here follow the handlers for each individual system call
For all of these system calls, a return value of zero or
greater indiciates success, and return of less than zero
indicates an error and the reason.
*/

int sys_debug(const char *str)
{
	if(!is_valid_string(str)) return KERROR_INVALID_ADDRESS;
	printf("%s", str);
	return 0;
}

int sys_process_yield()
{
	process_yield();
	return 0;
}

int sys_process_exit(int status)
{
	process_exit(status);
	return 0;
}

/* Helper routines to duplicate/free an argv array locally */

static char **argv_copy(int argc, const char **argv)
{
	char **pp;

	pp = kmalloc(sizeof(char *) * argc);
	int i;
	for(i = 0; i < argc; i++) {
		pp[i] = strdup(argv[i]);
	}

	return pp;
}

static void argv_delete(int argc, char **argv)
{
	int i;
	for(i = 0; i < argc; i++) {
		kfree(argv[i]);
	}
	kfree(argv);
}

/*
process_run() creates a child process in a more efficient
way than fork/exec by creating the child without duplicating
the memory state, then loading
*/

int sys_process_run(const char *path, int argc, const char **argv)
{
	if(!is_valid_path(path)) return KERROR_INVALID_PATH;

	/* Copy argv and path into kernel memory. */
	char **copy_argv = argv_copy(argc, argv);
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
		process_pass_arguments(p, argc, copy_argv);
	}

	/* SWITCH BACK TO ADDRESS SPACE OF PARENT PROCESS */
	current->pagetable = old_pagetable;
	pagetable_load(old_pagetable);

	/* Delete the argument and path copies. */
	argv_delete(argc, copy_argv);
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

/* Function creates a child process with the standard window replaced by wd */
int sys_process_wrun(const char *path, int argc, const char **argv, int *fds, int fd_len)
{
	if(!is_valid_path(path)) return KERROR_INVALID_PATH;

	/* Copy argv array into kernel memory. */
	char **copy_argv = argv_copy(argc, argv);
	char *copy_path = strdup(path);

	/* Create the child process */
	struct process *p = process_create();
	// process_inherit(current, p);
	process_selective_inherit(current, p, fds, fd_len);


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
		process_pass_arguments(p, argc, copy_argv);
	}

	/* SWITCH BACK TO ADDRESS SPACE OF PARENT PROCESS */
	current->pagetable = old_pagetable;
	pagetable_load(old_pagetable);

	/* Delete the argument copy. */
	argv_delete(argc, copy_argv);
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

int sys_process_exec(const char *path, int argc, const char **argv)
{
	if(!is_valid_path(path)) return KERROR_INVALID_PATH;

	addr_t entry;

	/* Duplicate the arguments into kernel space */
	char **copy_argv = argv_copy(argc, argv);

	/* Attempt to load the program image into this process. */
	int r = elf_load(current, path, &entry);

	/* On failure, return only if our address space is not corrupted. */
	if(r < 0) {
		if(r == KERROR_EXECUTION_FAILED) {
			process_kill(current->pid);
		}
		argv_delete(argc, copy_argv);
		return r;
	}

	/* Reset the stack and pass in the program arguments */
	process_stack_reset(current, PAGE_SIZE);
	process_kstack_reset(current, entry);
	process_pass_arguments(current, argc, copy_argv);

	/* Delete the local copy of the arguments. */
	argv_delete(argc, copy_argv);

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

int sys_process_reap(int pid)
{
	return process_reap(pid);
}

int sys_process_wait(struct process_info *info, int timeout)
{
	if(!is_valid_pointer(info,sizeof(*info))) return KERROR_INVALID_ADDRESS;
	return process_wait_child(0, info, timeout);
}

int sys_process_sleep(unsigned int ms)
{
	clock_wait(ms);
	return 0;
}

int sys_process_stats(struct process_stats *s, int pid)
{
	if(!is_valid_pointer(s,sizeof(*s))) return KERROR_INVALID_ADDRESS;
	return process_stats(pid, s);
}

int sys_process_heap(int delta)
{
	process_data_size_set(current, current->vm_data_size + delta);
	return PROCESS_ENTRY_POINT + current->vm_data_size;
}

int sys_object_list( int fd, char *buffer, int length)
{
	if(!is_valid_object(fd)) return KERROR_INVALID_OBJECT;
	if(!is_valid_pointer(buffer,length)) return KERROR_INVALID_ADDRESS;
	if(kobject_get_type(current->ktable[fd])!=KOBJECT_DIR) return KERROR_NOT_A_DIRECTORY;
	return kobject_read(current->ktable[fd],buffer,length);
}

int sys_open_file_relative( int fd, const char *path, int mode, kernel_flags_t flags)
{
	if(!is_valid_object(fd)) return KERROR_INVALID_OBJECT;
	if(!is_valid_path(path)) return KERROR_INVALID_PATH;

	int newfd = process_available_fd(current);
	if(newfd<0) return KERROR_OUT_OF_OBJECTS;

	struct kobject *newobj;
	int result = kobject_lookup(current->ktable[fd],path,&newobj);

	if(result>=0) {
		current->ktable[newfd] = newobj;
		return newfd;
	} else {
		return result;
	}
}

int sys_open_dir( int fd, const char *path, kernel_flags_t flags )
{
	if(!is_valid_object(fd)) return KERROR_INVALID_OBJECT;
	if(!is_valid_path(path)) return KERROR_INVALID_PATH;

       	int newfd = process_available_fd(current);
	if(newfd<0) return KERROR_OUT_OF_OBJECTS;

	struct kobject *newobj;
	int result;

	if(flags&KERNEL_FLAGS_CREATE) {
		newobj = kobject_create_dir_from_dir( current->ktable[fd], path );
		if(newobj) {
			result = 0;
		} else {
			result = KERROR_NOT_FOUND;
		}
	} else {
		result = kobject_lookup( current->ktable[fd], path, &newobj );
	}

	if(result>=0) {
		current->ktable[newfd] = newobj;
		return fd;
	} else {
		return result;
	}
}

int sys_open_file(const char *path, int mode, kernel_flags_t flags)
{
	if(!is_valid_path(path)) return KERROR_INVALID_PATH;

	int newfd = process_available_fd(current);
	if(newfd<0) return KERROR_OUT_OF_OBJECTS;

	struct fs_dirent *d = fs_resolve(path);

	if(fs_dirent_isdir(d)) {
		fs_dirent_close(d);
		return KERROR_NOT_A_FILE;
	}

	if(d) {
		current->ktable[newfd] = kobject_create_file(d);
		return newfd;
	} else {
		// XXX propagate better error
		return KERROR_NOT_FOUND;
	}
}

int sys_open_console(int wd)
{
	if(!is_valid_object_type(wd,KOBJECT_GRAPHICS)) return KERROR_INVALID_OBJECT;

	int fd = process_available_fd(current);
	if(fd<0) return KERROR_OUT_OF_OBJECTS;

	current->ktable[fd] = kobject_create_console_from_graphics(current->ktable[wd]);
	return fd;
}


int sys_open_window(int wd, int x, int y, int w, int h)
{
	if(!is_valid_object_type(wd,KOBJECT_GRAPHICS)) return KERROR_INVALID_OBJECT;

	struct kobject *k = current->ktable[wd];

	int fd = process_available_fd(current);
	if(fd<0) return KERROR_OUT_OF_OBJECTS;

	k = kobject_create_graphics_from_graphics(k,x,y,w,h);
	if(!k) {
		// XXX choose better errno
		return KERROR_INVALID_REQUEST;
	}

	current->ktable[fd] = k;

	return fd;
}

int sys_open_pipe()
{
	int fd = process_available_fd(current);
	if(fd < 0) {
		return KERROR_NOT_FOUND;
	}
	struct pipe *p = pipe_create();
	if(!p) {
		return KERROR_NOT_FOUND;
	}
	current->ktable[fd] = kobject_create_pipe(p);
	return fd;
}

int sys_object_type(int fd)
{
	if(!is_valid_object(fd)) return KERROR_INVALID_OBJECT;

	int fd_type = kobject_get_type(current->ktable[fd]);
	if(!fd_type)
		return 0;
	return fd_type;
}

// XXX the direction of dup here is backwards from the typical unix,
// which dups the second argument into the first.

int sys_object_dup(int fd1, int fd2)
{
	if(!is_valid_object(fd1)) return KERROR_INVALID_OBJECT;
	if(fd2>PROCESS_MAX_OBJECTS) return KERROR_INVALID_OBJECT;

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

int sys_object_read(int fd, void *data, int length)
{
	if(!is_valid_object(fd)) return KERROR_INVALID_OBJECT;
	if(!is_valid_pointer(data,length)) return KERROR_INVALID_ADDRESS;

	struct kobject *p = current->ktable[fd];
	return kobject_read(p, data, length);
}

int sys_object_read_nonblock(int fd, void *data, int length)
{
	if(!is_valid_object(fd)) return KERROR_INVALID_OBJECT;
	if(!is_valid_pointer(data,length)) return KERROR_INVALID_ADDRESS;

	struct kobject *p = current->ktable[fd];
	return kobject_read_nonblock(p, data, length);
}

int sys_object_write(int fd, void *data, int length)
{
	if(!is_valid_object(fd)) return KERROR_INVALID_OBJECT;
	if(!is_valid_pointer(data,length)) return KERROR_INVALID_ADDRESS;

	struct kobject *p = current->ktable[fd];
	return kobject_write(p, data, length);
}

int sys_object_seek(int fd, int offset, int whence)
{
	if(!is_valid_object(fd)) return KERROR_INVALID_OBJECT;

	// XXX add kobject method here
	return KERROR_NOT_IMPLEMENTED;
}

int sys_object_remove( int fd, const char *name )
{
	if(!is_valid_object(fd)) return KERROR_INVALID_OBJECT;
	if(!is_valid_path(name)) return KERROR_INVALID_PATH;
	return kobject_remove( current->ktable[fd], name );
}

int sys_object_close(int fd)
{
	if(!is_valid_object(fd)) return KERROR_INVALID_OBJECT;

	struct kobject *p = current->ktable[fd];
	kobject_close(p);
	current->ktable[fd] = 0;
	return 0;
}

int sys_object_set_tag(int fd, char *tag)
{
	if(!is_valid_object(fd)) return KERROR_INVALID_OBJECT;
	kobject_set_tag(current->ktable[fd], tag);
	return 0;
}

int sys_object_get_tag(int fd, char *buffer, int buffer_size)
{
	if(!is_valid_object(fd)) return KERROR_INVALID_OBJECT;
	return kobject_get_tag(current->ktable[fd], buffer, buffer_size);
}

int sys_object_set_blocking(int fd, int b)
{
	if(!is_valid_object(fd)) return KERROR_INVALID_OBJECT;
	return kobject_set_blocking(current->ktable[fd], b);
}

int sys_object_size(int fd, int *dims, int n)
{
	if(!is_valid_object(fd)) return KERROR_INVALID_OBJECT;
	if(!is_valid_pointer(dims,sizeof(*dims)*n)) return KERROR_INVALID_ADDRESS;

	struct kobject *p = current->ktable[fd];
	return kobject_size(p, dims, n);
}

int sys_object_copy( int src, int dst )
{
	if(!is_valid_object(src)) return KERROR_INVALID_OBJECT;
	if(!is_valid_object(dst)) return KERROR_INVALID_OBJECT;

	kobject_copy(current->ktable[src],&(current->ktable[dst]));

	return 0;
}

int sys_object_max()
{
	int max_fd = process_object_max(current);
	return max_fd;
}

int sys_system_stats(struct system_stats *s)
{
	if(!is_valid_pointer(s,sizeof(*s))) return KERROR_INVALID_ADDRESS;

	struct rtc_time t = { 0 };
	rtc_read(&t);
	s->time = rtc_time_to_timestamp(&t) - boottime;

	struct ata_count a = ata_stats();
	for(int i = 0; i < 4; i++) {
		s->blocks_written[i] = a.blocks_written[i];
		s->blocks_read[i] = a.blocks_read[i];
	}

	return 0;
}

int sys_bcache_stats(struct bcache_stats * s)
{
	if(!is_valid_pointer(s,sizeof(*s))) return KERROR_INVALID_ADDRESS;
	bcache_get_stats( s );
	return 0;
}

int sys_bcache_flush()
{
	bcache_flush_all();
	return 0;
}

int sys_system_time( uint32_t *tm )
{
	if(!is_valid_pointer(tm,sizeof(*tm))) return KERROR_INVALID_ADDRESS;
	struct rtc_time t;
	rtc_read(&t);
	*tm = rtc_time_to_timestamp(&t);
	return 0;
}

int sys_system_rtc( struct rtc_time *t )
{
	if(!is_valid_pointer(t,sizeof(*t))) return KERROR_INVALID_ADDRESS;
	rtc_read(t);
	return 0;
}

int sys_device_driver_stats(const char * name, struct device_driver_stats * stats)
{
	if(!is_valid_string(name)) return KERROR_INVALID_ADDRESS;
	if(!is_valid_pointer(stats,sizeof(*stats))) return KERROR_INVALID_ADDRESS;

	device_driver_get_stats(name, stats);
	return 0;
}

int sys_chdir(const char *path)
{
	if(!is_valid_path(path)) return KERROR_INVALID_PATH;

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

int32_t syscall_handler(syscall_t n, uint32_t a, uint32_t b, uint32_t c, uint32_t d, uint32_t e)
{
	if((n < MAX_SYSCALL) && current) {
		current->stats.syscall_count[n]++;
	}
	switch (n) {
	case SYSCALL_DEBUG:
		return sys_debug((const char *) a);
	case SYSCALL_PROCESS_YIELD:
		return sys_process_yield();
	case SYSCALL_PROCESS_EXIT:
		return sys_process_exit(a);
	case SYSCALL_PROCESS_RUN:
		return sys_process_run((const char *) a, b, (const char **)c);
	case SYSCALL_PROCESS_WRUN:
		return sys_process_wrun((const char *) a, b, (const char **) c, (int *) d, e);
	case SYSCALL_PROCESS_FORK:
		return sys_process_fork();
	case SYSCALL_PROCESS_EXEC:
		return sys_process_exec((const char *) a, b, (const char **) c);
	case SYSCALL_PROCESS_SELF:
		return sys_process_self();
	case SYSCALL_PROCESS_PARENT:
		return sys_process_parent();
	case SYSCALL_PROCESS_KILL:
		return sys_process_kill(a);
	case SYSCALL_PROCESS_REAP:
		return sys_process_reap(a);
	case SYSCALL_PROCESS_WAIT:
		return sys_process_wait((struct process_info *) a, b);
	case SYSCALL_PROCESS_SLEEP:
		return sys_process_sleep(a);
	case SYSCALL_PROCESS_STATS:
		return sys_process_stats((struct process_stats *) a, b);
	case SYSCALL_PROCESS_HEAP:
		return sys_process_heap(a);
	case SYSCALL_OPEN_FILE:
		return sys_open_file((const char *) a, b, c);
	case SYSCALL_OPEN_FILE_RELATIVE:
		return sys_open_file_relative(a, (const char *)b, c, d);
	case SYSCALL_OPEN_DIR:
		return sys_open_dir(a,(const char*)b,(kernel_flags_t)c);
	case SYSCALL_OPEN_WINDOW:
		return sys_open_window(a, b, c, d, e);
	case SYSCALL_OPEN_CONSOLE:
		return sys_open_console(a);
	case SYSCALL_OPEN_PIPE:
		return sys_open_pipe();
	case SYSCALL_OBJECT_TYPE:
		return sys_object_type(a);
	case SYSCALL_OBJECT_DUP:
		return sys_object_dup(a, b);
	case SYSCALL_OBJECT_READ:
		return sys_object_read(a, (void *) b, c);
	case SYSCALL_OBJECT_READ_NONBLOCK:
		return sys_object_read_nonblock(a, (void *) b, c);
	case SYSCALL_OBJECT_LIST:
		return sys_object_list(a, (char *) b, (int) c);
	case SYSCALL_OBJECT_WRITE:
		return sys_object_write(a, (void *) b, c);
	case SYSCALL_OBJECT_SEEK:
		return sys_object_seek(a, b, c);
	case SYSCALL_OBJECT_REMOVE:
		return sys_object_remove(a,(const char*)b);
	case SYSCALL_OBJECT_CLOSE:
		return sys_object_close(a);
	case SYSCALL_OBJECT_SET_TAG:
		return sys_object_set_tag(a, (char *) b);
	case SYSCALL_OBJECT_GET_TAG:
		return sys_object_get_tag(a, (char *) b, c);
	case SYSCALL_OBJECT_SET_BLOCKING:
		return sys_object_set_blocking(a, b);
	case SYSCALL_OBJECT_SIZE:
		return sys_object_size(a, (int *) b, c);
	case SYSCALL_OBJECT_MAX:
		return sys_object_max(a);
	case SYSCALL_OBJECT_COPY:
		return sys_object_copy(a,b);
	case SYSCALL_SYSTEM_STATS:
		return sys_system_stats((struct system_stats *) a);
	case SYSCALL_BCACHE_STATS:
		return sys_bcache_stats((struct bcache_stats *) a);
	case SYSCALL_BCACHE_FLUSH:
		return sys_bcache_flush();
	case SYSCALL_SYSTEM_TIME:
		return sys_system_time((uint32_t*)a);
	case SYSCALL_SYSTEM_RTC:
		return sys_system_rtc((struct rtc_time *) a);
	case SYSCALL_DEVICE_DRIVER_STATS:
		return sys_device_driver_stats((char *) a, (struct device_driver_stats *) b);
	case SYSCALL_CHDIR:
		return sys_chdir((const char *) a);
	default:
		return KERROR_INVALID_SYSCALL;
	}
}
