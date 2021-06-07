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

#define NWINDOWS 4

#define WINDOW_TITLE_HEIGHT 14
#define WINDOW_TITLE_COLOR 50,50,255
#define WINDOW_TITLE_TEXT_COLOR 255,255,255
#define WINDOW_BORDER_COLOR 200,200,200
#define WINDOW_BORDER 3
#define WINDOW_TEXT_PADDING 3

#define CLOSE_BOX_PADDING 2
#define CLOSE_BOX_SIZE (WINDOW_TITLE_HEIGHT-CLOSE_BOX_PADDING*2)
#define CLOSE_BOX_COLOR 100,100,100

struct window {
	int w,h,x,y;
	const char * exec;
	const char * arg;
	int argc;
	int pid;
	int fds[4];
};

void draw_border( struct window *win )
{
	int x=win->x;
	int y=win->y;
	int h=win->h;
	int w=win->w;

	// Title bar
	draw_color(WINDOW_TITLE_COLOR);
	draw_rect(x,y,w,WINDOW_TITLE_HEIGHT);

	// Close box
	draw_color(CLOSE_BOX_COLOR);
	draw_rect(x+CLOSE_BOX_PADDING,y+CLOSE_BOX_PADDING,CLOSE_BOX_SIZE,CLOSE_BOX_SIZE);
	// Title text
	draw_color(WINDOW_TITLE_TEXT_COLOR);
	draw_string(x+CLOSE_BOX_SIZE+CLOSE_BOX_PADDING*2,y+WINDOW_TEXT_PADDING,"Window Title");

	// Border box
	draw_color(WINDOW_BORDER_COLOR);
	draw_line(x,y,w,0);
	draw_line(x,y+WINDOW_TITLE_HEIGHT-1,w,0);

	draw_line(x,y,0,h);
	draw_line(x+1,y,0,h);

	draw_line(x,y+h,w,0);
	draw_line(x+1,y+h,w,0);

	draw_line(x+w,y,0,h);
	draw_line(x+w+1,y,0,h);

	// Recover old color here
	draw_color(255,255,255);
}

int main(int argc, char *argv[])
{
	int size[2];
	syscall_object_size(KNO_STDWIN, size, 2);
	draw_window(KNO_STDWIN);
	draw_clear(0, 0, size[0], size[1]);
	draw_flush();

	// Problem: this only works if it's a local variable!
struct window windows[NWINDOWS] = {
	{ .x=0, .y=0, .w = 400, .h = 400, .exec = "bin/shell.exe", .arg=0, .argc = 2 },
	{ .x=400, .y=0, .w = 400, .h = 400, .exec = "bin/saver.exe", .arg=0, .argc = 2 },
	{ .x=0, .y=400, .w = 400, .h = 300, .exec = "bin/snake.exe", .arg=0, .argc = 2 },
	{ .x=400, .y=400, .w = 400, .h = 300, .exec = "bin/fractal.exe", .arg=0, .argc = 2 },
};


	int i;
	for(i=0;i<NWINDOWS;i++) {
		struct window *w = &windows[i];

		w->fds[3] = syscall_open_window(KNO_STDWIN, w->x+WINDOW_BORDER, w->y+WINDOW_TITLE_HEIGHT, w->w-WINDOW_BORDER*2, w->h-WINDOW_BORDER-WINDOW_TITLE_HEIGHT);

		w->fds[0] = syscall_open_pipe();
		w->fds[1] = syscall_open_console(w->fds[3]);
		w->fds[2] = w->fds[1];

		const char *args[3];
		args[0] = w->exec;
		args[1] = w->arg;
		args[2] = 0;

		w->pid = syscall_process_wrun(w->exec, w->argc, args, w->fds, 4);
		if(w->pid<0) {
		  printf("couldn't exec %d\n",w->pid);
		  return 0;
		}

		draw_window(KNO_STDWIN);
		draw_color(255,255,255);
		draw_border(w);
		draw_flush();
	}

	/* Finally, allow the user to switch between windows*/
	int active = 0;
	char tin = 0;

	/* Draw green window around active process and start it */
	draw_window(KNO_STDWIN);
	draw_color(0,255,0);
	draw_border(&windows[active]);
	draw_flush();

	while (tin != '~') {
		if (windows[active].pid == 0) {
			active = (active + 1) % NWINDOWS;
			continue;
		}

		/* If tab entered, go to the next process */
		syscall_object_read(0, &tin, 1);
		if (tin == '\t') {
			draw_window(KNO_STDWIN);
			draw_color(255,255,255);
			draw_border(&windows[active]);
			draw_flush();
			active = (active + 1) % NWINDOWS;

			/* Draw green window around active process and start it */
			draw_window(KNO_STDWIN);
			draw_color(0,255,0);
			draw_border(&windows[active]);
			draw_flush();
			// need to return to prior color
			draw_color(255, 255, 255);
			continue;
		}
		/* Write 1 character to the correct pipe */
		syscall_object_write(windows[active].fds[0], &tin, 1);
	}

	/* Reap all children processes */
	for (i=0;i<NWINDOWS;i++) {
		syscall_process_reap(windows[i].pid);
	}

	/* Clean up the window */
	draw_color(255, 255, 255);
	draw_clear(0, 0, size[0], size[1]);
	draw_flush();
	return 0;
}

