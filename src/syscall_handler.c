/*
Copyright (C) 2015 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/

#include "syscall.h"
#include "syscall_handler.h"
#include "console.h"
#include "keyboard.h"
#include "process.h"
#include "kmalloc.h"
#include "cdromfs.h"
#include "memorylayout.h"
#include "graphics_lib.h"
#include "main.h"
#include "fs.h"
#include "kobject.h"
#include "pagetable.h"
#include "clock.h"
#include "rtc.h"
#include "elf.h"
#include "kmalloc.h"

// Get rid of this once we have a proper dirlist stream
#define LSDIR_TEMP_BUFFER_SIZE 250

int sys_debug( const char *str )
{
	console_printf("%s",str);
	return 0;
}

int sys_exit( int status )
{
	process_exit(status);
	return 0;
}

int sys_yield()
{
	process_yield();
	return 0;
}

int sys_sbrk (int a)
{
    unsigned int vaddr = (unsigned int) current->brk;
    unsigned int paddr;
    unsigned int i;
    for (i = 0; i < (unsigned int) a; i += PAGE_SIZE){
        if (!pagetable_getmap(current->pagetable, vaddr, &paddr))
        {
            pagetable_alloc(current->pagetable, vaddr, PAGE_SIZE, PAGE_FLAG_USER | PAGE_FLAG_READWRITE);
        }
        vaddr += PAGE_SIZE;
    }
    if (!pagetable_getmap(current->pagetable, vaddr, &paddr))
    {
        pagetable_alloc(current->pagetable, vaddr, PAGE_SIZE, PAGE_FLAG_USER | PAGE_FLAG_READWRITE);
    }
    vaddr = (unsigned int) current->brk;
    current->brk += a;
    return vaddr;
}

/*
sys_process_run creates a new child process running the executable named by "path".
In this temporary implementation, we use the cdrom filesystem directly.
(Instead, we should go through the abstract filesystem interface.)
Takes in argv and argc for the new process' main
*/

int sys_process_run( const char *path, const char** argv, int argc )
{
	struct process *p = elf_load(path, 0);
    
    if (!p) {
        return ENOENT;
    }

    process_inherit(p);
    process_pass_arguments(p, argv, argc);


	/* Put the new process into the ready list */

	process_launch(p);

	return p->pid;
}

void sys_exec(const char * path, const char ** argv, int argc) {
	struct process *p = elf_load(path, current->pid);

  if (!p) {
      return;
  }

  process_inherit(p);
  process_pass_arguments(p, argv, argc);
	process_launch(p);
  current = p;
	process_yield(); // Otherwise we will jump back into the old process
}

int sys_fork()
{
  struct process *p = process_create(0, 0, 0);
  p->state = PROCESS_STATE_FORK_CHILD;
  p->ppid = current->pid;
  pagetable_delete(p->pagetable);
  p->pagetable = pagetable_duplicate(current->pagetable);
  process_inherit(p);
  process_launch(p);
  process_fork_freeze();
  return p->pid;
}

uint32_t sys_gettimeofday()
{
	struct rtc_time t;
	rtc_read(&t);
	return rtc_time_to_timestamp(&t);
}

int sys_mount(uint32_t device_no, const char *fs_name, const char *ns)
{
	struct fs *fs = fs_get(fs_name);
	struct fs_volume *v = fs_volume_mount(fs, device_no);
	int ret = process_mount_as(current, v, ns);
	kfree(fs);
	return ret;
}

int sys_chdir(const char *ns, const char *name)
{
	return process_chdir(current, ns, name);
}

int sys_mkdir(const char *name){
	struct fs_dirent *cwd = current->cwd;
	return fs_dirent_mkdir(cwd, name);
}

int sys_readdir(const char *name, char *buffer, int len){
	struct fs_dirent *cwd = current->cwd;
	struct fs_dirent *d = fs_dirent_namei(cwd, name);
	if (!d) return -1;
	return fs_dirent_readdir(d, buffer, len);
}

int sys_rmdir(const char *name){
	struct fs_dirent *cwd = current->cwd;
	struct fs_dirent *d = fs_dirent_namei(cwd, name);
	if (!d) return -1;
	return fs_dirent_rmdir(cwd, name);
}

int sys_open( const char *path, int mode, int flags )
{
	int fd = process_available_fd(current);
	int ret = 0;
	if (fd < 0)
		return -1;
	struct fs_dirent *cwd = current->cwd;
	struct fs_dirent *d = fs_dirent_namei(cwd, path);
	if (!d) {
		ret = fs_dirent_mkfile(cwd, path);
		d = fs_dirent_namei(cwd, path);
	}
	struct fs_file *fp = fs_file_open(d, mode);
	current->ktable[fd] = kobject_create_file(fp);
	return fd;
}

int sys_keyboard_read_char()
{
	return keyboard_read();
}

int sys_read( int fd, void *data, int length )
{
	struct kobject *p = current->ktable[fd];
    return kobject_read(p, data, length);

}

int sys_write( int fd, void *data, int length )
{
	struct kobject *p = current->ktable[fd];
	return kobject_write(p, data, length);
}

int sys_lseek( int fd, int offset, int whence )
{
	return ENOSYS;
}

int sys_close( int fd )
{
	struct kobject *p = current->ktable[fd];
	kobject_close(p);
	current->ktable[fd] = 0;
	return 0;
}

int sys_pwd(char *result)
{
	struct fs_dirent *d = current->cwd;
	char dir_list[LSDIR_TEMP_BUFFER_SIZE];
	memset(dir_list, 0, LSDIR_TEMP_BUFFER_SIZE);
	result[0] = 0;
	while (1) {
		struct fs_dirent *parent = fs_dirent_namei(d, "..");
		int hit_root, found_child;
		fs_dirent_compare(parent, d, &hit_root);
		if (hit_root) {
			kfree(parent);
			break;
		}
		if (fs_dirent_readdir(parent, dir_list, LSDIR_TEMP_BUFFER_SIZE) < 0) return -1;
		char *dir = strtok(dir_list, " ");
		while (dir) {
			struct fs_dirent *child = fs_dirent_namei(parent, dir);
			fs_dirent_compare(child, d, &found_child);
			if (found_child) {
				char result_next[LSDIR_TEMP_BUFFER_SIZE];
				memset(result_next, 0, LSDIR_TEMP_BUFFER_SIZE);
				strcat(result_next, "/");
				strcat(result_next, dir);
				strcat(result_next, result);
				if (strlen(result) + strlen(result_next) + 1 > LSDIR_TEMP_BUFFER_SIZE) return -1;
				strcpy(result, result_next);
				break;
			}
			dir = strtok(dir + strlen(dir) + 1, " ");
		}
		d = parent;
	}
	return 0;
}

int sys_draw_color( int wd, int r, int g, int b ) {
    if (wd < 0 || wd >= current->window_count) {
        return ENOENT;
    }
    struct graphics_color c;
    c.r = r;
    c.g = g;
    c.b = b;
    c.a = 0;
    graphics_fgcolor( current->windows[wd], c );
    return 0;
}

int sys_draw_rect( int wd, int x, int y, int w, int h ) {
    if (wd < 0 || wd >= current->window_count) {
        return ENOENT;
    }
    graphics_rect( current->windows[wd], x, y, w, h );
    return 0;
}

int sys_draw_clear( int wd, int x, int y, int w, int h ) {
    if (wd < 0 || wd >= current->window_count) {
        return ENOENT;
    }
    graphics_clear( current->windows[wd], x, y, w, h );
    return 0;
}

int sys_draw_line( int wd, int x, int y, int w, int h ) {
    if (wd < 0 || wd >= current->window_count) {
        return ENOENT;
    }
    graphics_line( current->windows[wd], x, y, w, h );
    return 0;
}

int sys_draw_char( int wd, int x, int y, char c ) {
    if (wd < 0 || wd >= current->window_count) {
        return ENOENT;
    }
    graphics_char( current->windows[wd], x, y, c );
    return 0;
}

int sys_draw_string( int wd, int x, int y, char *s ) {
    if (wd < 0 || wd >= current->window_count) {
        return ENOENT;
    }
    int i;
    for (i = 0; s[i]; i++) {
        graphics_char( current->windows[wd], x+i*8, y, s[i] );
    }
    return 0;
}

int sys_draw_create( int wd, int x, int y, int w, int h ) {
    if (current->window_count >= PROCESS_MAX_WINDOWS || wd < 0 || wd >= current->window_count || current->windows[wd]->clip.w < x + w || current->windows[wd]->clip.h < y + h) {
        return ENOENT;
    }

    current->windows[current->window_count] = graphics_create(current->windows[wd]);

    if (!current->windows[current->window_count]) {
        return ENOENT;
    }

    current->windows[current->window_count]->clip.x = x + current->windows[wd]->clip.x;
    current->windows[current->window_count]->clip.y = y + current->windows[wd]->clip.y;
    current->windows[current->window_count]->clip.w = w;
    current->windows[current->window_count]->clip.h = h;

    return current->window_count++;
}

int sys_draw_write( struct graphics_command *s ) {
    return graphics_write(s);
}

int sys_sleep(unsigned int ms)
{
	clock_wait(ms);
	return 0;
}

int sys_process_self()
{
	return process_getpid();
}

int sys_process_parent()
{
	return process_getppid();
}

int sys_process_kill( int pid )
{
	return process_kill(pid);
}

int sys_process_wait( struct process_info *info, int timeout )
{
	return process_wait_child(info, timeout);
}

int sys_process_reap( int pid )
{
	return process_reap(pid);
}

int32_t syscall_handler( syscall_t n, uint32_t a, uint32_t b, uint32_t c, uint32_t d, uint32_t e )
{
	switch(n) {
	case SYSCALL_EXIT:	return sys_exit(a);
	case SYSCALL_DEBUG:	return sys_debug((const char*)a);
	case SYSCALL_YIELD:	return sys_yield();
	case SYSCALL_OPEN:	return sys_open((const char *)a,b,c);
	case SYSCALL_READ:	return sys_read(a,(void*)b,c);
	case SYSCALL_WRITE:	return sys_write(a,(void*)b,c);
	case SYSCALL_LSEEK:	return sys_lseek(a,b,c);
	case SYSCALL_CLOSE:	return sys_close(a);
	case SYSCALL_KEYBOARD_READ_CHAR:	return sys_keyboard_read_char();
	case SYSCALL_DRAW_COLOR:	return sys_draw_color(a, b, c, d);
	case SYSCALL_DRAW_RECT:	return sys_draw_rect(a, b, c, d, e);
	case SYSCALL_DRAW_LINE:	return sys_draw_line(a, b, c, d, e);
	case SYSCALL_DRAW_CLEAR:	return sys_draw_clear(a, b, c, d, e);
	case SYSCALL_DRAW_CHAR:	return sys_draw_char(a, b, c, (char)d);
	case SYSCALL_DRAW_STRING:	return sys_draw_string(a, b, c, (char*)d);
	case SYSCALL_DRAW_CREATE:	return sys_draw_create(a, b, c, d, e);
	case SYSCALL_DRAW_WRITE:	return sys_draw_write((struct graphics_command*)a);
	case SYSCALL_SLEEP:	return sys_sleep(a);
	case SYSCALL_GETTIMEOFDAY:	return sys_gettimeofday();
	case SYSCALL_MOUNT:	return sys_mount(a, (const char *) b, (const char *) c);
	case SYSCALL_CHDIR:	return sys_chdir((const char *) a, (const char *) b);
	case SYSCALL_SBRK: return sys_sbrk (a);
	case SYSCALL_PROCESS_SELF:	return sys_process_self();
	case SYSCALL_PROCESS_PARENT:	return sys_process_parent();
	case SYSCALL_PROCESS_RUN:	return sys_process_run((const char *)a, (const char**)b, c);
	case SYSCALL_FORK:	return sys_fork();
	case SYSCALL_EXEC:	sys_exec((const char *)a, (const char **)b, c);
	case SYSCALL_PROCESS_KILL:	return sys_process_kill(a);
	case SYSCALL_PROCESS_WAIT:	return sys_process_wait((struct process_info*)a, b);
	case SYSCALL_PROCESS_REAP:	return sys_process_reap(a);
	case SYSCALL_MKDIR:	return sys_mkdir((const char *)a);
	case SYSCALL_READDIR:	return sys_readdir((const char *)a, (char *) b, (int) c);
	case SYSCALL_RMDIR:	return sys_rmdir((const char *)a);
	case SYSCALL_PWD:	return sys_pwd((char *)a);
	default:		return -1;
	}
}

