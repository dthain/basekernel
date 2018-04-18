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
#include "user-io.h"

int main( const char *argv[], int argc )
{
	uint32_t j = 0;
	printf("mounting\n");
	int res = mount(0, "kevin", "K");
	printf("mounted successfully? %d\n", res);
	ns_change("K");
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
		strcat(buffer,num);
		strcat(buffer,newline);
		n = write(fd, buffer, strlen(buffer));
		if (n < 0) break;
		printf("wrote %d chars: %s\n", n, buffer);
	}
	close(fd);
	fd = open("kevin", 1, 0);
	char buffer[1000];
	int n;
	printf("reading file...\n");
	while ((n = read(fd, buffer, 999)) > 0) {
		buffer[n] = 0;
		printf("%s");
		flush();
	}
	close(fd);
	exit(0);
  
	return 0;
}
