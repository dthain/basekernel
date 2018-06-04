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
	int fd = open("kevin", 2, 0);
	printf("got fd %d\n", fd);
	printf("writing to file...\n");
	for(;;) {
		j++;
		char buffer[100] = "Hello, world! I can write to ";
		char num[5];
		char newline[2] = "\n";
		int n;
		uint_to_string(j, num);
		strcat(buffer, num);
		strcat(buffer, newline);
		n = write(fd, buffer, strlen(buffer));
		if(n < 0)
			break;
		printf("wrote %d chars: %s\n", n, buffer);
	}
	close(fd);
	fd = open("kevin", 1, 0);
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
