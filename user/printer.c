/*
Copyright (C) 2016-2019 The University of Notre Dame
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

int main(int argc, char *argv[])
{
	draw_window(KNO_STDWIN);
	draw_color(0, 0, 255);
	draw_clear(0, 0, WIDTH, HEIGHT);
	draw_flush();

	int cd = syscall_open_console(KNO_STDWIN);
	if(cd < 0) {
		printf("Console open failed!\n");
		return 2;
	}
	syscall_object_dup(cd, KNO_STDOUT);

	printf("hello world!\n");

	return 0;
}
