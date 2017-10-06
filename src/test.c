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

    int wd = w_create(399, 398, 160, 16);
    if (wd < 0) {
        debug("Window create failed!");
        exit(1);
    }
	for(j=0;j<10;j++) {
		debug("hello world!\n");
        w_color(wd, 255, 255, 255);
        w_rect(wd, 0, 0, 154, 12);
        w_line(wd, 2, 14, 152, 1);
        w_string(wd, 1, 2, "hi, this is a test.");
     		for(i=0;i<100000000;i++)  {}
	}

    w_color(wd, 255, 0, 0);
    w_clear(wd, 0, 0, 160, 16);

	exit(0);

	return 0;
}
