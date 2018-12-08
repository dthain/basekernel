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
	//int fd = open_intent("/bin/open.exe", 0, 0);
	int fd = open("/bin/open.exe", 0, 0);
	char buffer[1000];
	int n;
	printf("reading file...\n");
	while((n = read(fd, buffer, 10)) > 0) {
		buffer[n] = 0;
		printf("About to print...\n");
		printf("%s", buffer);
		flush();
	}
	close(fd);
	process_exit(0);

	return 0;
}
