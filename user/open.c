/*
Copyright (C) 2017 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/

/*
A trivial user level program to try out basic system calls.
*/

#include "library/syscalls.h"
#include "library/string.h"
#include "library/user-io.h"

int main(const char *argv[], int argc)
{
	uint32_t j = 0;
	chdir("/");
	printf("got root\n");
	int fd = open("/bin/open.exe", 1, 0);
	char buffer[1000];
	int n;
	printf("reading file...\n");
	while((n = read(fd, buffer, 999)) > 0) {
		buffer[n] = 0;
		printf("%s");
		flush();
	}
	close(fd);
	process_exit(0);

	return 0;
}
