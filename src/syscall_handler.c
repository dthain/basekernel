/*
Copyright (C) 2015 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/

#include "syscall.h"
#include "console.h"
#include "process.h"

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

int sys_fork()
{
	return ENOSYS;
}

int sys_exec( const char *cmd )
{
	return ENOSYS;
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
	case SYSCALL_FORK:	return sys_fork();
	case SYSCALL_EXEC:	return sys_exec((const char *)a);
	case SYSCALL_WAIT:	return sys_wait();
	case SYSCALL_OPEN:	return sys_open((const char *)a,b,c);
	case SYSCALL_READ:	return sys_read(a,(void*)b,c);
	case SYSCALL_WRITE:	return sys_write(a,(void*)b,c);
	case SYSCALL_LSEEK:	return sys_lseek(a,b,c);
	case SYSCALL_CLOSE:	return sys_close(a);
	default:		return -1;
	}
}

