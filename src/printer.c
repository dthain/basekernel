/*
Copyright (C) 2017 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/

/*
A fun graphics demo that features a line segment bouncing around the screen.
*/

#include "syscalls.h"
#include "string.h"
#include "user-io.h"

#define WIDTH    (200)
#define HEIGHT   (200)

int main( const char *argv[], int argc )
{
    int wd = draw_create(0, 600, 300, WIDTH, HEIGHT);
    if (wd < 0) {
        debug("Window create failed!\n");
        exit(1);
    }
    int cd = console_open(wd);
    if (cd < 0) {
        debug("Console open failed!\n");
        exit(2);
    }
    printf("wd: %d\ncd: %d\n", wd, cd);

    draw_window(wd);
    draw_color(0, 0, 255);
    draw_clear(0, 0, WIDTH, HEIGHT);
    draw_flush();

    write(cd, "hello\nworld", 12);
	exit(0);

	return 0;
}

