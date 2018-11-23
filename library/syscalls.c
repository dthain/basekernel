/*
Copyright (C) 2017 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/

#include "kernel/syscall.h"
#include "kernel/stats.h"
#include "kernel/gfxstream.h"

void debug(const char *str)
{
	syscall(SYSCALL_DEBUG, (uint32_t) str, 0, 0, 0, 0);
}

void process_exit(int status)
{
	syscall(SYSCALL_PROCESS_EXIT, status, 0, 0, 0, 0);
}

int process_yield()
{
	return syscall(SYSCALL_PROCESS_YIELD, 0, 0, 0, 0, 0);
}

int open(const char *path, int mode, int flags)
{
	return syscall(SYSCALL_OPEN, (uint32_t) path, mode, flags, 0, 0);
}

int object_type(int fd)
{
	return syscall(SYSCALL_OBJECT_TYPE, fd, 0, 0, 0, 0);
}

int dup(int fd1, int fd2)
{
	return syscall(SYSCALL_DUP, fd1, fd2, 0, 0, 0);
}

int read(int fd, void *data, int length)
{
	return syscall(SYSCALL_READ, fd, (uint32_t) data, length, 0, 0);
}

int read_nonblock(int fd, void *data, int length)
{
	return syscall(SYSCALL_READ_NONBLOCK, fd, (uint32_t) data, length, 0, 0);
}


int write(int fd, void *data, int length)
{
	return syscall(SYSCALL_WRITE, fd, (uint32_t) data, length, 0, 0);
}

int lseek(int fd, int offset, int whence)
{
	return syscall(SYSCALL_LSEEK, fd, offset, whence, 0, 0);
}

int close(int fd)
{
	return syscall(SYSCALL_CLOSE, fd, 0, 0, 0, 0);
}

extern void *sbrk(int a)
{
	return (void *) syscall(SYSCALL_SBRK, a, 0, 0, 0, 0);
}

int pipe_open()
{
	return syscall(SYSCALL_OPEN_PIPE, 0, 0, 0, 0, 0);
}

int set_blocking(int fd, int b)
{
	return syscall(SYSCALL_SET_BLOCKING, fd, b, 0, 0, 0);
}

int open_window(int wd, int x, int y, int w, int h)
{
	return syscall(SYSCALL_OPEN_WINDOW, wd, x, y, w, h);
}

int process_sleep(unsigned int ms)
{
	return syscall(SYSCALL_PROCESS_SLEEP, ms, 0, 0, 0, 0);
}

uint32_t gettimeofday()
{
	return syscall(SYSCALL_GETTIMEOFDAY, 0, 0, 0, 0, 0);
}

int process_self()
{
	return syscall(SYSCALL_PROCESS_SELF, 0, 0, 0, 0, 0);
}

int process_parent()
{
	return syscall(SYSCALL_PROCESS_PARENT, 0, 0, 0, 0, 0);
}

int process_run(const char *cmd, const char **argv, int argc)
{
	return syscall(SYSCALL_PROCESS_RUN, (uint32_t) cmd, (uint32_t) argv, argc, 0, 0);
}

int process_fork()
{
	return syscall(SYSCALL_PROCESS_FORK, 0, 0, 0, 0, 0);
}

void process_exec(const char *path, const char **argv, int argc)
{
	syscall(SYSCALL_PROCESS_EXEC, (uint32_t) path, (uint32_t) argv, (uint32_t) argc, 0, 0);
}

int process_kill(unsigned int pid)
{
	return syscall(SYSCALL_PROCESS_KILL, pid, 0, 0, 0, 0);
}

int console_open(int wd)
{
	return syscall(SYSCALL_OPEN_CONSOLE, wd, 0, 0, 0, 0);
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

int process_reap(unsigned int pid)
{
	return syscall(SYSCALL_PROCESS_REAP, pid, 0, 0, 0, 0);
}

int process_wait(struct process_info *info, int timeout)
{
	return syscall(SYSCALL_PROCESS_WAIT, (uint32_t) info, timeout, 0, 0, 0);
}

int sys_stats(struct sys_stats *s)
{
	return syscall(SYSCALL_SYS_STATS, (uint32_t) s, 0, 0, 0, 0);
}

int process_stats(struct proc_stats *s, unsigned int pid)
{
	return syscall(SYSCALL_PROCESS_STATS, (uint32_t) s, pid, 0, 0, 0);
}

int get_dimensions(int fd, int * dims, int n)
{
	return syscall(SYSCALL_GET_DIMENSIONS, fd, (uint32_t) dims, n, 0, 0);
}

int process_wrun(const char *cmd, const char **argv, int argc, int wd)
{
	return syscall(SYSCALL_PROCESS_WRUN, (uint32_t) cmd, (uint32_t) argv, argc, wd, 0);
}