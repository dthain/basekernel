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


	for(j=0;j<10;j++) {
		debug("hello world!\n");
        w_color(0, 255, 255, 255);
        w_rect(0, 399, 398, 154, 12);
        w_line(0, 400, 412, 152, 1);
        w_string(0, 400, 400, "hi, this is a test.");
     		for(i=0;i<100000000;i++)  {}
	}

    w_color(0, 255, 0, 0);
    w_clear(0, 399, 398, 157, 16);

	exit(0);

	return 0;
}
