/*
Copyright (C) 2016-2019 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/

/*
Simple window manager runs a list of programs and distributes
events to each based on which one currently has the focus.
*/

#include "library/syscalls.h"
#include "library/string.h"
#include "library/user-io.h"
#include "library/kernel_object_string.h"
#include "library/nanowin.h"

#define NWINDOWS 4

#define WINDOW_TITLE_HEIGHT 14
#define WINDOW_TITLE_ACTIVE_COLOR 100,100,255
#define WINDOW_TITLE_INACTIVE_COLOR 25,25,50
#define WINDOW_TITLE_TEXT_COLOR 255,255,255
#define WINDOW_BORDER_COLOR 200,200,200
#define WINDOW_BORDER 3
#define WINDOW_TEXT_PADDING 3

#define CLOSE_BOX_PADDING 3
#define CLOSE_BOX_SIZE (WINDOW_TITLE_HEIGHT-CLOSE_BOX_PADDING*2)
#define CLOSE_BOX_COLOR 100,100,100

struct window {
	int w,h,x,y;
	int console_mode;
	const char * exec;
	const char * arg;
	int argc;
	int pid;
	int fds[4];
};

struct nwindow *nw = 0;

void draw_border( struct window *win, int isactive )
{
	int x=win->x;
	int y=win->y;
	int h=win->h;
	int w=win->w;

	// Title bar
	if(isactive) {
		nw_bgcolor(nw,WINDOW_TITLE_ACTIVE_COLOR);
	} else {
		nw_bgcolor(nw,WINDOW_TITLE_INACTIVE_COLOR);
	}
	nw_clear(nw,x,y,w,WINDOW_TITLE_HEIGHT);

	// Close box
	nw_fgcolor(nw,CLOSE_BOX_COLOR);
	nw_rect(nw,x+CLOSE_BOX_PADDING,y+CLOSE_BOX_PADDING,CLOSE_BOX_SIZE,CLOSE_BOX_SIZE);
	// Title text
	nw_fgcolor(nw,WINDOW_TITLE_TEXT_COLOR);
	nw_string(nw,x+CLOSE_BOX_SIZE+CLOSE_BOX_PADDING*2,y+WINDOW_TEXT_PADDING,win->exec);

	// Border box
	nw_fgcolor(nw,WINDOW_BORDER_COLOR);
	nw_line(nw,x,y,w,0);
	nw_line(nw,x,y+WINDOW_TITLE_HEIGHT-1,w,0);

	nw_line(nw,x,y,0,h);
	nw_line(nw,x+1,y,0,h);

	nw_line(nw,x,y+h,w,0);
	nw_line(nw,x+1,y+h,w,0);

	nw_line(nw,x+w,y,0,h);
	nw_line(nw,x+w+1,y,0,h);

	nw_bgcolor(nw,0,0,0);
}

int main(int argc, char *argv[])
{
	nw = nw_create_default();

	nw_clear(nw,0, 0, nw_width(nw), nw_height(nw));
	nw_flush(nw);

	struct event e;

	struct window windows[NWINDOWS] = {
		{ .x=0,         .y=0,         .console_mode=1, .exec = "bin/shell.exe", .arg=0, .argc = 2 },
		{ .x=nw_width(nw)/2, .y=0,         .console_mode=0, .exec = "bin/saver.exe", .arg=0, .argc = 2 },
		{ .x=0,         .y=nw_height(nw)/2, .console_mode=0, .exec = "bin/snake.exe", .arg=0, .argc = 2 },
		{ .x=nw_width(nw)/2, .y=nw_height(nw)/2, .console_mode=1, .exec = "bin/fractal.exe", .arg=0, .argc = 2 },
	};

	int i;
	for(i=0;i<NWINDOWS;i++) {
		windows[i].w = nw_width(nw)/2-2;
		windows[i].h = nw_height(nw)/2-2;
	}

	for(i=0;i<NWINDOWS;i++) {
		struct window *w = &windows[i];

		struct nwindow *child = nw_create_child(nw,w->x+WINDOW_BORDER, w->y+WINDOW_TITLE_HEIGHT, w->w-WINDOW_BORDER*2, w->h-WINDOW_BORDER-WINDOW_TITLE_HEIGHT);

		w->fds[3] = nw_fd(child);

		if(w->console_mode) {
			w->fds[0] = syscall_open_console(w->fds[3]);
			w->fds[1] = w->fds[0];
			w->fds[2] = w->fds[0];
		} else {
			w->fds[0] = w->fds[3];
			w->fds[1] = w->fds[3];
			w->fds[2] = w->fds[3]; // Not right place for stderr...
		}

		const char *args[3];
		args[0] = w->exec;
		args[1] = w->arg;
		args[2] = 0;

		w->pid = syscall_process_wrun(w->exec, w->argc, args, w->fds, 4);
		if(w->pid<0) {
			printf("couldn't exec %d\n",w->pid);
			return 0;
		}

		draw_border(w,0);
		nw_flush(nw);
	}

	/* Finally, allow the user to switch between windows*/
	int active = 0;

	/* Draw green window around active process */
	draw_border(&windows[active],1);
	nw_flush(nw);

	//struct event e;
	while (nw_next_event(nw,&e)) {

		if(e.type==EVENT_CLOSE) break;
		if(e.type!=EVENT_KEY_DOWN) continue;

		char c = e.code;

		if (c == '\t') {
			/* If tab entered, go to the next process */

			/* Draw white boundary around old window. */
			draw_border(&windows[active],0);
			nw_flush(nw);
			active = (active + 1) % NWINDOWS;

			/* Draw green window around new window. */
			draw_border(&windows[active],1);
			nw_flush(nw);
		} else if (c=='~') {
			/* If tilde entered, cancel the whole thing. */
			break;
		} else {
			if(windows[active].console_mode) {
				// Post a single character to the console.
				syscall_object_write(windows[active].fds[0],&c,1,KERNEL_IO_POST);
			} else {
				// Post a complete event to the window.
				syscall_object_write(windows[active].fds[0],&e,sizeof(e),KERNEL_IO_POST);
			}
		}
	}

	/* Reap all children processes */
	for (i=0;i<NWINDOWS;i++) {
		syscall_process_reap(windows[i].pid);
	}

	/* XXX should kill child process here */

	/* Clean up the window */
	nw_clear(nw, 0, 0, nw_width(nw), nw_height(nw));
	nw_flush(nw);
	return 0;
}

