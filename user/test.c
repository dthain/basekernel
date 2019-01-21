/*
Copyright (C) 2016-2019 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/

#include "library/syscalls.h"
#include "library/string.h"
#include "library/malloc.h"

int main(int argc, char *argv[])
{
	printf("hello world, I am %d, and I have %d arguments!\n", syscall_process_self(), argc);

	printf("They are: ");

	int i;
	for(i = 0; i < argc; ++i) {
		printf("(%s) ", argv[i]);
	}
	printf("\n");

	int n = 4096;
	printf("adding up %d values...\n", n);

	int *big = malloc(n * sizeof(int));
	if(!big)
		printf("malloc failed!\n");

	for(i = 0; i < n; i++) {
		big[i] = i;
	}
	int sum = 0;
	for(i = 0; i < n; ++i) {
		sum += big[i];
	}
	free(big);

	printf("The sum of 0 to %d is %d\n", n, sum);

	return 0;
}
