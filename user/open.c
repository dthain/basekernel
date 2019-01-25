/*
Copyright (C) 2016-2019 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/

#include "library/syscalls.h"
#include "library/string.h"
#include "library/user-io.h"

int main(int argc, char *argv[])
{
	syscall_chdir("/");
	printf("got root\n");
	int dir_fd = syscall_open_file("/", 0, 0);
	syscall_object_set_tag(dir_fd, "ROOT");
	printf("Opened root directory\n");
	int fd = syscall_open_file("ROOT:/data/words", 0, 0);
	char buffer[1000];
	int n;
	printf("reading file...\n");
	while((n = syscall_object_read(fd, buffer, 100)) > 0) {
		buffer[n] = 0;
		printf("%s", buffer);
		flush();
	}
	syscall_object_close(fd);
	syscall_process_exit(0);

	return 0;
}
