/*
Copyright (C) 2017 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/

#include "syscall.h"

void debug( const char *str ) {
	syscall( SYSCALL_DEBUG, (uint32_t) str, 0, 0, 0, 0 );
}

void exit( int status ) {
	syscall( SYSCALL_EXIT, status, 0, 0, 0, 0 );
}

int yield() {
	return syscall( SYSCALL_YIELD, 0, 0, 0, 0, 0 );
}

int run( const char *cmd ) {
	return syscall( SYSCALL_RUN, (uint32_t) cmd, 0, 0, 0, 0 );
}

int wait() {
	return syscall( SYSCALL_WAIT, 0, 0, 0, 0, 0 );
}

int open( const char *path, int mode, int flags ) {
	return syscall( SYSCALL_OPEN, (uint32_t) path, mode, flags, 0, 0 );
}

int read( int fd, void *data, int length ) {
	return syscall( SYSCALL_READ, fd, (uint32_t) data, length, 0, 0 );
}

int write( int fd, void *data, int length ) {
	return syscall( SYSCALL_WRITE, fd, (uint32_t) data, length, 0, 0 );
}

int lseek( int fd, int offset, int whence ) {
	return syscall( SYSCALL_LSEEK, fd, offset, whence, 0, 0 );
}

int close( int fd ) {
	return syscall( SYSCALL_CLOSE, fd, 0, 0, 0, 0 );
}

int draw_color( int wd, int r, int g, int b ) {
	return syscall( SYSCALL_DRAW_COLOR, wd, r, g, b, 0 );
}

int draw_rect( int wd, int x, int y, int w, int h ) {
	return syscall( SYSCALL_DRAW_RECT, wd, x, y, w, h );
}

int draw_clear( int wd, int x, int y, int w, int h ) {
	return syscall( SYSCALL_DRAW_CLEAR, wd, x, y, w, h );
}

int draw_line( int wd, int x, int y, int w, int h ) {
	return syscall( SYSCALL_DRAW_LINE, wd, x, y, w, h );
}

int draw_char( int wd, int x, int y, char c ) {
	return syscall( SYSCALL_DRAW_CHAR, wd, x, y, c, 0 );
}

int draw_string( int wd, int x, int y, char *s ) {
	return syscall( SYSCALL_DRAW_STRING, wd, x, y, (uint32_t)s, 0 );
}

int draw_create( int wd, int x, int y, int w, int h ) {
	return syscall( SYSCALL_DRAW_CREATE, wd, x, y, w, h );
}
