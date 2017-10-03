/*
Copyright (C) 2017 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/

#ifndef SYSCALL_H
#define SYSCALL_H

#include "kerneltypes.h"

typedef enum {
	SYSCALL_DEBUG,
	SYSCALL_YIELD,
	SYSCALL_RUN,
	SYSCALL_WAIT,
	SYSCALL_EXIT,
	SYSCALL_OPEN,
	SYSCALL_READ,
	SYSCALL_WRITE,
	SYSCALL_LSEEK,
	SYSCALL_CLOSE,
	SYSCALL_GETPID,
	SYSCALL_GETPPID
} syscall_t;

typedef enum {
	ENOENT=-1,
	EINVAL=-2,
	EACCES=-3,
	ENOSYS=-4
} syscall_error_t;

uint32_t syscall( syscall_t s, uint32_t a, uint32_t b, uint32_t c, uint32_t d, uint32_t e );

#endif
