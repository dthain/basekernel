/*
Copyright (C) 2017 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/

#ifndef SYSCALLS_H
#define SYSCALLS_H

#include "kerneltypes.h"

void debug( const char *str );
void exit( int status );
int yield();
int run( const char *cmd );
int wait();
int open( const char *path, int mode, int flags );
int read( int fd, void *data, int length );
int write( int fd, void *data, int length );
int lseek( int fd, int offset, int whence );
int close( int fd );
int draw_color( int wd, int r, int g, int b );
int draw_rect( int wd, int x, int y, int w, int h );
int draw_clear( int wd, int x, int y, int w, int h );
int draw_line( int wd, int x, int y, int w, int h );
int draw_char( int wd, int x, int y, char c );
int draw_string( int wd, int x, int y, char *s );
int draw_create( int wd, int x, int y, int w, int h );
int mount(uint32_t device_no, const char *fs_name, const char *ns);
int chdir(const char *ns, const char *path);
int sleep( unsigned int ms );
uint32_t gettimeofday();
int getpid();
int getppid();

#endif
