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
#include "malloc.h"

int main( const char *argv[], int argc )
{
    printf("hello world, I am %d, and I have %d arguments!\n", process_self(), argc);

    printf("They are: ");

    int i;
    for (i = 0; i < argc; ++i) {
        printf("(%s) ", argv[i]);
    }
    printf("\n");

    int* big = malloc(4097*sizeof(int));
    for (i = 0; i <= 100; ++i) {
        big[i] = i;
    }

    int* sum = big+4097;
    *sum = 0;
    for (i = 0; i <= 100; ++i) {
        *sum += big[i];
    }
    free(big);

    printf("The sum of 0 to 100 is %d\n", *sum);

	return 0;
}
