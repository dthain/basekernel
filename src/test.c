/*
Copyright (C) 2017 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/

/*
A trivial user level program to try out basic system calls.
This program requires that write() and exit() work correctly.
*/

#include "syscalls.h"

int main( const char *argv[], int argc )
{
	int i, j;

    int wd = draw_create(0, 399, 398, 160, 16);
    if (wd < 0) {
        debug("Window create failed!\n");
        exit(1);
    }
	for(j=0;j<10;j++) {
		debug("hello world!\n");
        draw_color(wd, 255, 255, 255);
        draw_rect(wd, 0, 0, 154, 12);
        draw_line(wd, 2, 14, 152, 1);
        draw_string(wd, 1, 2, "hi, this is a test.");
     		for(i=0;i<100000000;i++)  {}
	}

    draw_color(wd, 255, 0, 0);
    draw_clear(wd, 0, 0, 160, 16);
	exit(0);

	return 0;
}
