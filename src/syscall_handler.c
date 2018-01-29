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
#include "cdromfs.h"
#include "memorylayout.h"
#include "graphics_lib.h"
#include "main.h"
#include "fs.h"
#include "clock.h"
#include "rtc.h"
#include "kmalloc.h"

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


/*
sys_process_run creates a new child process running the executable named by "path".
In this temporary implementation, we use the cdrom filesystem directly.
(Instead, we should go through the abstract filesystem interface.)
Takes in argv and argc for the new process' main
*/

int sys_process_run( const char *path, const char** argv, int argc )
{
	/* Open and find the named path, if it exists. */

	if(!root_directory) return ENOENT;

	struct fs_dirent *d = fs_dirent_namei(root_directory,path);
	if(!d) {
		return ENOENT;
	}

	int length = d->sz;

	/* Create a new process with enough pages for the executable and one page for the stack */

	struct process *p = process_create(length,PAGE_SIZE*2);
	if(!p) return ENOENT;

	/* Round up length of the executable to an even pages */

	int i;
	int npages = length/PAGE_SIZE + (length%PAGE_SIZE ? 1 : 0);

	struct fs_file *f = fs_file_open(d, FS_FILE_READ);

	/* For each page, load one page from the file.  */
	/* Notice that the cdrom block size (2048) is half the page size (4096) */

	for(i=0;i<npages;i++) {
		unsigned vaddr = PROCESS_ENTRY_POINT + i * PAGE_SIZE;
		unsigned paddr;

		pagetable_getmap(p->pagetable,vaddr,&paddr);
		fs_file_read(f,(void*)paddr, PAGE_SIZE);
	}

	/* Close everything up */
	
	fs_dirent_close(d);
	fs_file_close(f);

    /* Copy open windows */
    memcpy(p->windows, current->windows, sizeof(p->windows));
    p->window_count = current->window_count;
    for(i=0;i<p->window_count;i++) {
        p->windows[i]->count++;
    }
    process_pass_arguments(p, argv, argc);

  
    /* Set the parent of the new process to the calling process */
    p->ppid = process_getpid();

	/* Put the new process into the ready list */

	process_launch(p);

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
	int ret = process_mount_as(v, ns);
	kfree(fs);
	return ret;
}

int sys_chdir(const char *ns, const char *name)
{
	return process_chdir(ns, name);
}

int sys_open( const char *path, int mode, int flags )
{
	int fd = process_available_fd();
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
	current->fdtable[fd] = fp;
	return fd;
}

int sys_keyboard_read_char()
{
	return keyboard_read();
}

int sys_read( int fd, void *data, int length )
{
	struct fs_file *fp = current->fdtable[fd];
	return fs_file_read(fp, data, length);
}

int sys_write( int fd, void *data, int length )
{
	struct fs_file *fp = current->fdtable[fd];
	return fs_file_write(fp, data, length);
}

int sys_lseek( int fd, int offset, int whence )
{
	return ENOSYS;
}

int sys_close( int fd )
{
	struct fs_file *fp = current->fdtable[fd];
	fs_file_close(fp);
	current->fdtable[fd] = 0;
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
	case SYSCALL_PROCESS_SELF:	return sys_process_self();
	case SYSCALL_PROCESS_PARENT:	return sys_process_parent();
	case SYSCALL_PROCESS_RUN:	return sys_process_run((const char *)a, (const char**)b, c);
	case SYSCALL_PROCESS_KILL:	return sys_process_kill(a);
	case SYSCALL_PROCESS_WAIT:	return sys_process_wait((struct process_info*)a, b);
	case SYSCALL_PROCESS_REAP:	return sys_process_reap(a);
	default:		return -1;
	}
}

