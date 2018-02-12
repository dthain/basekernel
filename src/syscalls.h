/*
Copyright (C) 2017 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/

#ifndef SYSCALLS_H
#define SYSCALLS_H

#include "kerneltypes.h"
#include "graphics_lib.h"

void debug( const char *str );
void exit( int status );
int yield();
int process_run( const char *cmd, const char** argv, int argc );
int process_run_subset( const char *cmd, const char** argv, int argc, int wd );
int open( const char *path, int mode, int flags );
int read( int fd, void *data, int length );
int write( int fd, void *data, int length );
int lseek( int fd, int offset, int whence );
int close( int fd );
char keyboard_read_char();
int draw_create( int wd, int x, int y, int w, int h );
void draw_write( struct graphics_command *s );
int sleep( unsigned int ms );
uint32_t gettimeofday();
int process_self();
int process_parent();
int process_kill( unsigned int pid );
int process_reap( unsigned int pid );
int process_wait( struct process_info* info, int timeout );

#endif
