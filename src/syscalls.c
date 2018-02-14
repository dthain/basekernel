/*
Copyright (C) 2017 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/

#include "syscall.h"
#include "graphics_lib.h"

void debug( const char *str ) {
	syscall( SYSCALL_DEBUG, (uint32_t) str, 0, 0, 0, 0 );
}

void exit( int status ) {
	syscall( SYSCALL_EXIT, status, 0, 0, 0, 0 );
}

int yield() {
	return syscall( SYSCALL_YIELD, 0, 0, 0, 0, 0 );
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

char keyboard_read_char() {
	return syscall( SYSCALL_KEYBOARD_READ_CHAR, 0, 0, 0, 0, 0 );
}

int draw_create( int wd, int x, int y, int w, int h ) {
	return syscall( SYSCALL_DRAW_CREATE, wd, x, y, w, h );
}

void draw_write( struct graphics_command *s ) {
	syscall( SYSCALL_DRAW_WRITE, (uint32_t) s, 0, 0, 0, 0 );
}

int sleep( unsigned int ms ) {
	return syscall( SYSCALL_SLEEP, ms, 0, 0, 0, 0 );
}

uint32_t gettimeofday() {
	return syscall(SYSCALL_GETTIMEOFDAY, 0, 0, 0, 0, 0);
}

int process_self() {
    static int cache = 0;
    return cache? (cache) : (cache=syscall( SYSCALL_PROCESS_SELF, 0, 0, 0, 0, 0 ));
}

int process_parent() {
    static int cache = 0;
    return cache? (cache) : (cache=syscall( SYSCALL_PROCESS_PARENT, 0, 0, 0, 0, 0 ));
}

int process_run( const char *cmd, const char** argv, int argc ) {
	return syscall( SYSCALL_PROCESS_RUN, (uint32_t) cmd, (uint32_t) argv, argc, 0, 0 );
}

int process_kill( unsigned int pid ) {
    return syscall( SYSCALL_PROCESS_KILL, pid, 0, 0, 0, 0 );
}

int mount(uint32_t device_no, const char *fs_name, const char *ns)
{
	return syscall(SYSCALL_MOUNT, device_no, (uint32_t) fs_name, (uint32_t) ns, 0, 0);
}

int chdir(const char *ns, const char *path)
{
	return syscall(SYSCALL_CHDIR, (uint32_t) ns, (uint32_t) path, 0, 0, 0);
}

int getpid() {
    static int cache = 0;
    return cache? (cache) : (cache=syscall( SYSCALL_GETPID, 0, 0, 0, 0, 0 ));
}

int process_reap( unsigned int pid ) {
    return syscall( SYSCALL_PROCESS_REAP, pid, 0, 0, 0, 0 );
}

int process_wait( struct process_info* info, int timeout ) {
    return syscall( SYSCALL_PROCESS_WAIT, (uint32_t)info, timeout, 0, 0, 0 );
}
