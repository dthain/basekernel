/*
Copyright (C) 2015 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/

#ifndef SYSCALL_H
#define SYSCALL_H

#include "kerneltypes.h"

typedef enum {
	SYSCALL_DEBUG,
	SYSCALL_YIELD,
	SYSCALL_FORK,
	SYSCALL_EXEC,
	SYSCALL_WAIT,
	SYSCALL_EXIT,
	SYSCALL_OPEN,
	SYSCALL_READ,
	SYSCALL_WRITE,
	SYSCALL_LSEEK,
	SYSCALL_CLOSE
} syscall_t;

typedef enum {
	ENOENT,
	EINVAL,
	EACCES,
	ENOSYS
} syscall_error_t;

uint32_t syscall( syscall_t s, uint32_t a, uint32_t b, uint32_t c, uint32_t d, uint32_t e );

static inline void debug( const char *str )
{ syscall( SYSCALL_DEBUG, (uint32_t) str, 0, 0, 0, 0 ); }

static inline void exit( int status )
	{ syscall( SYSCALL_EXIT, status, 0, 0, 0, 0 ); }

static inline int yield()
	{ return syscall( SYSCALL_YIELD, 0, 0, 0, 0, 0 ); }

static inline int fork()
	{ return syscall( SYSCALL_FORK, 0, 0, 0, 0, 0 ); }

static inline int exec( const char *cmd )
	{ return syscall( SYSCALL_EXEC, (uint32_t) cmd, 0, 0, 0, 0 ); }

static inline int wait()
	{ return syscall( SYSCALL_WAIT, 0, 0, 0, 0, 0 ); }

static inline int open( const char *path, int mode, int flags )
	{ return syscall( SYSCALL_OPEN, (uint32_t) path, mode, flags, 0, 0 ); }

static inline int read( int fd, void *data, int length )
	{ return syscall( SYSCALL_READ, fd, (uint32_t) data, length, 0, 0 ); }

static inline int write( int fd, void *data, int length )
	{ return syscall( SYSCALL_WRITE, fd, (uint32_t) data, length, 0, 0 ); }

static inline int lseek( int fd, int offset, int whence )
	{ return syscall( SYSCALL_LSEEK, fd, offset, whence, 0, 0 ); }

static inline int close( int fd )
{ return syscall( SYSCALL_CLOSE, fd, 0, 0, 0, 0 ); }

#endif
