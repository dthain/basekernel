/*
Copyright (C) 2017 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/

#include "syscall.h"
#include "graphics_lib.h"

static int self_pid_cache = 0;
static int self_ppid_cache = 0;

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

int dup( int fd1, int fd2 ) {
	return syscall( SYSCALL_DUP, fd1, fd2, 0, 0, 0 );
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

extern void* sbrk( int a ) {
	return (void*) syscall( SYSCALL_SBRK, a, 0, 0, 0, 0 );
}

int keyboard_read_char() {
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
	return self_pid_cache? (self_pid_cache) : (self_pid_cache=syscall( SYSCALL_PROCESS_SELF, 0, 0, 0, 0, 0 )); 
}

int process_parent() {
    return self_ppid_cache? (self_ppid_cache) : (self_ppid_cache=syscall( SYSCALL_PROCESS_PARENT, 0, 0, 0, 0, 0 ));
}

int process_run( const char *cmd, const char** argv, int argc ) {
	return syscall( SYSCALL_PROCESS_RUN, (uint32_t) cmd, (uint32_t) argv, argc, 0, 0 );
}

int fork() {
	self_pid_cache = 0;
	self_ppid_cache = 0;
	int cpid = syscall( SYSCALL_FORK, 0, 0, 0, 0, 0 );
	if (cpid == process_self()) return 0;
	return cpid;
}

void exec(const char * path, const char ** argv, int argc) {
	syscall( SYSCALL_EXEC, (uint32_t)path, (uint32_t)argv, (uint32_t)argc, 0, 0 );
}

int ns_copy(const char *old_ns, const char * new_ns) {
	return syscall( SYSCALL_NS_COPY, (uint32_t)old_ns, (uint32_t)new_ns, 0, 0, 0 );
}

int ns_delete(const char *ns) {
	return syscall( SYSCALL_NS_DELETE, (uint32_t)ns, 0, 0, 0, 0 );
}

int ns_get_perms(const char *ns) {
	return syscall( SYSCALL_NS_GET_PERMS, (uint32_t)ns, 0, 0, 0, 0 );
}

int ns_remove_perms(const char *ns, int mask) {
	return syscall( SYSCALL_NS_GET_PERMS, (uint32_t)ns, mask, 0, 0, 0 );
}

int ns_lower_root(const char *ns, const char * path) {
	return syscall( SYSCALL_NS_GET_PERMS, (uint32_t)ns, (uint32_t)path, 0, 0, 0 );
}

int ns_change(const char *ns) {
	return syscall( SYSCALL_NS_CHANGE, (uint32_t)ns, 0, 0, 0, 0 );
}

int process_kill( unsigned int pid ) {
    return syscall( SYSCALL_PROCESS_KILL, pid, 0, 0, 0, 0 );
}

int console_open( int wd ) {
	return syscall( SYSCALL_OPEN_CONSOLE, wd, 0, 0, 0, 0 );
}

int mount(uint32_t device_no, const char *fs_name, const char *ns)
{
	return syscall(SYSCALL_MOUNT, device_no, (uint32_t) fs_name, (uint32_t) ns, 0, 0);
}

int chdir(const char *ns, const char *path)
{
	return syscall(SYSCALL_CHDIR, (uint32_t) ns, (uint32_t) path, 0, 0, 0);
}

int mkdir(const char *name)
{
	return syscall(SYSCALL_MKDIR, (uint32_t) name, 0, 0, 0, 0);
}

int rmdir(const char *name)
{
	return syscall(SYSCALL_RMDIR, (uint32_t) name, 0, 0, 0, 0);
}

int readdir(const char *name, char *buffer, int n)
{
	return syscall(SYSCALL_READDIR, (uint32_t) name, (uint32_t) buffer, (uint32_t) n, 0, 0);
}

int pwd(char *buffer)
{
	return syscall(SYSCALL_PWD, (uint32_t) buffer, 0, 0, 0, 0);
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
