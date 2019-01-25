/*
Copyright (C) 2016-2019 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/

/*
A trivial user level program to try out basic system calls.
*/

#include "library/syscalls.h"
#include "library/string.h"
#include "library/user-io.h"

int main(int argc, char *argv[])
{
	syscall_chdir("/");
	printf("got root\n");
	int fd = syscall_open_file("kevin", 2, 0);
	printf("got fd %d\n", fd);
	printf("writing to file...\n");
	for(int i = 0; i < 12; i++) {
		char buffer[4096] = {'a'+i};
		int n;
		n = syscall_object_write(fd, buffer, strlen(buffer));
		if(n < 0)
			break;
		printf("wrote %d chars: %s\n", n, buffer);
	}
	syscall_object_close(fd);
	fd = syscall_open_file("kevin", 1, 0);
	printf("reading file...\n");
	for (int i = 0; i < 12; i++) {
		char buffer[4096];
		int n;
		while((n = syscall_object_read(fd, buffer, 4096-1)) > 0) {
			buffer[n] = 0;
			printf("%s");
			flush();
		}
	}
	syscall_object_close(fd);
	syscall_process_exit(0);

	return 0;
}
