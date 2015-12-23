/*
Copyright (C) 2015 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file COPYING for details.
*/

#include "syscall.h"
#include "console.h"
#include "process.h"

uint32_t sys_exit( uint32_t code )
{
	process_exit(code);
	return 0;
}

uint32_t sys_yield()
{
	process_yield();
	return 0;
}

uint32_t sys_testcall( uint32_t code )
{
	console_printf("testing: %d\n",code);
	return 0;
}

int32_t syscall_handler( uint32_t n, uint32_t a, uint32_t b, uint32_t c, uint32_t d, uint32_t e )
{
	switch(n) {
	case SYSCALL_exit:	return sys_exit(a);
	case SYSCALL_testcall:	return sys_testcall(a);
	case SYSCALL_yield:	return sys_yield();
	default:		return -1;
	}
}

