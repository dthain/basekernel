/*
Copyright (C) 2015 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/

#include "syscall.h"
#include "console.h"
#include "process.h"
#include "cdromfs.h"
#include "memorylayout.h"
#include "main.h"
#include "fs.h"

int sys_debug( const char *str )
{
	console_printf("%s",str);
	return 0;
}

int sys_exit( int status )
{
	process_exit(status);
	return 0;
}

int sys_yield()
{
	process_yield();
	return 0;
}


/*
sys_run creates a new child process running the executable named by "path".
In this temporary implementation, we use the cdrom filesystem directly.
(Instead, we should go through the abstract filesystem interface.)
*/

int sys_run( const char *path )
{
	/* Open and find the named path, if it exists. */

	if(!root_directory) return ENOENT;

	struct dirent *d = fs_lookup(root_directory,path);
	if(!d) {
		return ENOENT;
	}

	int length = d->sz;

	/* Create a new process with enough pages for the executable and one page for the stack */

	struct process *p = process_create(length,PAGE_SIZE);
	if(!p) return ENOENT;

	/* Round up length of the executable to an even pages */

	int i;
	int npages = length/PAGE_SIZE + length%PAGE_SIZE ? 1 : 0;

	struct file *f = fs_open(d, 0);

	/* For each page, load one page from the file.  */
	/* Notice that the cdrom block size (2048) is half the page size (4096) */

	for(i=0;i<npages;i++) {
		unsigned vaddr = PROCESS_ENTRY_POINT + i * PAGE_SIZE;
		unsigned paddr;

		pagetable_getmap(p->pagetable,vaddr,&paddr);
		fs_read(f,(void*)paddr, PAGE_SIZE);
	}

	/* Close everything up */
	
	fs_dirent_close(d);
	fs_close(f);

	/* Put the new process into the ready list */

	process_launch(p);

	return 0;
}

int sys_wait()
{
	return ENOSYS;
}

int sys_open( const char *path, int mode, int flags )
{
	return ENOSYS;
}

int sys_read( int fd, void *data, int length )
{
	return ENOSYS;
}

int sys_write( int fd, void *data, int length )
{
	return ENOSYS;
}

int sys_lseek( int fd, int offset, int whence )
{
	return ENOSYS;
}

int sys_close( int fd )
{
	return ENOSYS;
}

int32_t syscall_handler( syscall_t n, uint32_t a, uint32_t b, uint32_t c, uint32_t d, uint32_t e )
{
	switch(n) {
	case SYSCALL_EXIT:	return sys_exit(a);
	case SYSCALL_DEBUG:	return sys_debug((const char*)a);
	case SYSCALL_YIELD:	return sys_yield();
	case SYSCALL_RUN:	return sys_run((const char *)a);
	case SYSCALL_WAIT:	return sys_wait();
	case SYSCALL_OPEN:	return sys_open((const char *)a,b,c);
	case SYSCALL_READ:	return sys_read(a,(void*)b,c);
	case SYSCALL_WRITE:	return sys_write(a,(void*)b,c);
	case SYSCALL_LSEEK:	return sys_lseek(a,b,c);
	case SYSCALL_CLOSE:	return sys_close(a);
	default:		return -1;
	}
}

