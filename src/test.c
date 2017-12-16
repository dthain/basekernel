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

int main( const char *argv[], int argc )
{
	printf("Hello world, I am %d, and I have %d arguments!\n", process_self(), argc);
	printf("They are: ");

	int i;
	for (i = 0; i < argc; ++i) {
		printf("(%s) ", argv[i]);
	}

	printf("\n");
	return 0;
}
