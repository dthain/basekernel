/*
Copyright (C) 2017 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/

#ifndef SYSCALLS_H
#define SYSCALLS_H

#include "kernel/types.h"
#include "kernel/stats.h"
#include "kernel/gfxstream.h"

void debug(const char *str);
void process_exit(int status);
int process_yield();
int process_run(const char *cmd, const char **argv, int argc);
int process_fork();
void process_exec(const char *path, const char **argv, int argc);
int process_self();
int process_parent();
int process_kill(unsigned int pid);
int process_reap(unsigned int pid);
int process_wait(struct process_info *info, int timeout);
int process_sleep(unsigned int ms);
int process_object_max();
int open(const char *path, int mode, int flags);
int object_type(int fd);
int dup(int fd1, int fd2);
int read(int fd, void *data, int length);
int copy_volume(int srcdisk, int dstdisk, char *srcfs, char *destfs);
int read_nonblock(int fd, void *data, int length);
int write(int fd, void *data, int length);
int lseek(int fd, int offset, int whence);
int close(int fd);
int object_set_intent(int fd, char *intent);
int object_get_intent(int fd, char *buffer, int buffer_size);
extern void *sbrk(int a);
char keyboard_read_char();
int open_window(int wd, int x, int y, int w, int h);
int set_blocking(int fd, int b);
int console_open(int fd);
int pipe_open();
int mkdir(const char *path);
int readdir(const char *path, char *buffer, int buffer_len);
int rmdir(const char *path);
int pwd(char *buffer);
int chdir(const char *path);
int sys_stats(struct sys_stats *s);
int process_stats(struct proc_stats *s, unsigned int pid);
uint32_t gettimeofday();
void gettimeofday_rtc(struct rtc_time * time);
int get_dimensions(int fd, int * dims, int n);
int process_wrun(const char *cmd, const char **argv, int argc, int * fds, int fd_len);

#endif
