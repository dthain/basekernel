/*
Copyright (C) 2017 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/

#include "kernel/syscall.h"
#include "kernel/stats.h"
#include "kernel/gfxstream.h"

void syscall_debug(const char *str)
{
	syscall(SYSCALL_DEBUG, (uint32_t) str, 0, 0, 0, 0);
}

void syscall_process_exit(int status)
{
	syscall(SYSCALL_PROCESS_EXIT, status, 0, 0, 0, 0);
}

int syscall_process_yield()
{
	return syscall(SYSCALL_PROCESS_YIELD, 0, 0, 0, 0, 0);
}

int syscall_process_run(const char *cmd, const char **argv, int argc)
{
	return syscall(SYSCALL_PROCESS_RUN, (uint32_t) cmd, (uint32_t) argv, argc, 0, 0);
}

int syscall_process_wrun(const char *cmd, const char **argv, int argc, int * fds, int fd_len)
{
	return syscall(SYSCALL_PROCESS_WRUN, (uint32_t) cmd, (uint32_t) argv, argc, (uint32_t) fds, fd_len);
}

int syscall_process_fork()
{
	return syscall(SYSCALL_PROCESS_FORK, 0, 0, 0, 0, 0);
}

void syscall_process_exec(const char *path, const char **argv, int argc)
{
	syscall(SYSCALL_PROCESS_EXEC, (uint32_t) path, (uint32_t) argv, (uint32_t) argc, 0, 0);
}

int syscall_process_self()
{
	return syscall(SYSCALL_PROCESS_SELF, 0, 0, 0, 0, 0);
}

int syscall_process_parent()
{
	return syscall(SYSCALL_PROCESS_PARENT, 0, 0, 0, 0, 0);
}

int syscall_process_kill(unsigned int pid)
{
	return syscall(SYSCALL_PROCESS_KILL, pid, 0, 0, 0, 0);
}

int syscall_process_reap(unsigned int pid)
{
	return syscall(SYSCALL_PROCESS_REAP, pid, 0, 0, 0, 0);
}

int syscall_process_wait(struct process_info *info, int timeout)
{
	return syscall(SYSCALL_PROCESS_WAIT, (uint32_t) info, timeout, 0, 0, 0);
}

int syscall_process_sleep(unsigned int ms)
{
	return syscall(SYSCALL_PROCESS_SLEEP, ms, 0, 0, 0, 0);
}

int syscall_process_stats(struct process_stats *s, unsigned int pid)
{
	return syscall(SYSCALL_PROCESS_STATS, (uint32_t) s, pid, 0, 0, 0);
}

extern void *syscall_process_heap(int a)
{
	return (void *) syscall(SYSCALL_PROCESS_HEAP, a, 0, 0, 0, 0);
}

int syscall_open_file(const char *path, int mode, int flags)
{
	return syscall(SYSCALL_OPEN_FILE, (uint32_t) path, mode, flags, 0, 0);
}

int syscall_open_file_relative( int fd, const char *path, int mode, int flags)
{
	return syscall(SYSCALL_OPEN_FILE_RELATIVE, fd, (uint32_t) path, mode, flags, 0);
}

int syscall_open_window(int wd, int x, int y, int w, int h)
{
	return syscall(SYSCALL_OPEN_WINDOW, wd, x, y, w, h);
}

int syscall_open_console(int wd)
{
	return syscall(SYSCALL_OPEN_CONSOLE, wd, 0, 0, 0, 0);
}

int syscall_open_pipe()
{
	return syscall(SYSCALL_OPEN_PIPE, 0, 0, 0, 0, 0);
}

int syscall_object_type(int fd)
{
	return syscall(SYSCALL_OBJECT_TYPE, fd, 0, 0, 0, 0);
}

int syscall_object_dup(int fd1, int fd2)
{
	return syscall(SYSCALL_OBJECT_DUP, fd1, fd2, 0, 0, 0);
}

int syscall_object_read(int fd, void *data, int length)
{
	return syscall(SYSCALL_OBJECT_READ, fd, (uint32_t) data, length, 0, 0);
}

int syscall_object_read_nonblock(int fd, void *data, int length)
{
	return syscall(SYSCALL_OBJECT_READ_NONBLOCK, fd, (uint32_t) data, length, 0, 0);
}

int syscall_object_readdir( int fd, char *buffer, int n)
{
	return syscall(SYSCALL_OBJECT_READDIR, fd, (uint32_t) buffer, (uint32_t) n, 0, 0);
}

int syscall_object_write(int fd, void *data, int length)
{
	return syscall(SYSCALL_OBJECT_WRITE, fd, (uint32_t) data, length, 0, 0);
}

int syscall_object_seek(int fd, int offset, int whence)
{
	return syscall(SYSCALL_OBJECT_SEEK, fd, offset, whence, 0, 0);
}

int syscall_object_close(int fd)
{
	return syscall(SYSCALL_OBJECT_CLOSE, fd, 0, 0, 0, 0);
}

int syscall_object_set_intent(int fd, char *intent)
{
	return syscall(SYSCALL_OBJECT_SET_INTENT, fd, (uint32_t)intent, 0, 0, 0);
}

int syscall_object_get_intent(int fd, char *buffer, int buffer_size)
{
	return syscall(SYSCALL_OBJECT_GET_INTENT, fd, (uint32_t)buffer, buffer_size, 0, 0);
}

int syscall_object_set_blocking(int fd, int b)
{
	return syscall(SYSCALL_OBJECT_SET_BLOCKING, fd, b, 0, 0, 0);
}

int syscall_object_size(int fd, int *dims, int n)
{
	return syscall(SYSCALL_OBJECT_SIZE, fd, (uint32_t) dims, n, 0, 0);
}

int syscall_object_max()
{
	return syscall(SYSCALL_OBJECT_MAX, 0, 0, 0, 0, 0);
}

int syscall_copy_volume(int srcdisk, int dstdisk, char *srcfs, char *destfs)
{
	return syscall(SYSCALL_COPY_VOLUME, srcdisk, dstdisk, (uint32_t) srcfs, (uint32_t) destfs, 0);
}

int syscall_system_stats(struct system_stats *s)
{
	return syscall(SYSCALL_SYSTEM_STATS, (uint32_t) s, 0, 0, 0, 0);
}

int syscall_system_time( uint32_t *t )
{
  return syscall(SYSCALL_SYSTEM_TIME, (uint32_t)t, 0, 0, 0, 0);
}

int syscall_system_rtc( struct rtc_time *time )
{
	return syscall(SYSCALL_SYSTEM_RTC, (uint32_t)time, 0, 0, 0, 0);
}

int syscall_mkdir(const char *name)
{
	return syscall(SYSCALL_MKDIR, (uint32_t) name, 0, 0, 0, 0);
}

int syscall_rmdir(const char *name)
{
	return syscall(SYSCALL_RMDIR, (uint32_t) name, 0, 0, 0, 0);
}

int syscall_chdir(const char *path)
{
	return syscall(SYSCALL_CHDIR, (uint32_t) path, 0, 0, 0, 0);
}


