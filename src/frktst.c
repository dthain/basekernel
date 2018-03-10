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
    int x = fork();
    printf("hello world, I am %d.\n", process_self());
    printf("My fork returned %d\n", x);

    printf("Some numbers:\n");

    int i;
    for (i = 0; i < 5; ++i) {
        printf("%d\n", i);
    }
    printf("\n");

	return 0;
}
