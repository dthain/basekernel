/*
Copyright (C) 2018 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/

/*
A trivial user level program to try out basic system calls.
*/

#include "library/syscalls.h"
#include "library/string.h"
#include "library/user-io.h"

int main(int argc, const char *argv[])
{
	chdir("/");
	printf("got root\n");
	int fd = open("kevin", 2, 0);
	printf("got fd %d\n", fd);
	printf("writing to file...\n");
	for(int i = 0; i < 12; i++) {
		char buffer[4096] = {'a'+i};
		int n;
		n = write(fd, buffer, strlen(buffer));
		if(n < 0)
			break;
		printf("wrote %d chars: %s\n", n, buffer);
	}
	close(fd);
	fd = open("kevin", 1, 0);
	printf("reading file...\n");
	for (int i = 0; i < 12; i++) {
		char buffer[4096];
		int n;
		while((n = read(fd, buffer, 4096-1)) > 0) {
			buffer[n] = 0;
			printf("%s");
			flush();
		}
	}
	close(fd);
	process_exit(0);

	return 0;
}
