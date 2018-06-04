/*
Copyright (C) 2017 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/

/*
A fun graphics demo that features a line segment bouncing around the screen.
*/

#include "library/syscalls.h"
#include "library/string.h"
#include "library/user-io.h"

#define WIDTH    (200)
#define HEIGHT   (200)

int main(const char *argv[], int argc)
{
	int wd = open_window(KNO_STDWIN, 600, 300, WIDTH, HEIGHT);
	if(wd < 0) {
		debug("Window create failed!\n");
		return 1;
	}
	draw_window(wd);
	draw_color(0, 0, 255);
	draw_clear(0, 0, WIDTH, HEIGHT);
	draw_flush();

	int cd = console_open(wd);
	if(cd < 0) {
		debug("Console open failed!\n");
		return 2;
	}
	dup(cd, KNO_STDOUT);

	printf("hello world!\n");

	return 0;
}
