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
#include "string.h"
#include "userio.h"

int main( const char *argv[], int argc )
{
	int j;

	for(j=0;j<10;j++) {
		printf("hello world, I can count to %d!\n", j);
		sleep(1000);
	}

	exit(0);

	return 0;
}
