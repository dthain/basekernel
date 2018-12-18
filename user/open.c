/*
Copyright (C) 2017 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/

#include "library/syscalls.h"
#include "library/string.h"
#include "library/user-io.h"

int main(const char *argv[], int argc)
{
	chdir("/");
	printf("got root\n");
	int dir_fd = open("/", 0, 0);
	object_set_intent(dir_fd, "ROOT");
	printf("Opened root directory\n");
	int fd = open("ROOT:/data/words", 0, 0);
	char buffer[1000];
	int n;
	printf("reading file...\n");
	while((n = read(fd, buffer, 100)) > 0) {
		buffer[n] = 0;
		printf("%s", buffer);
		flush();
	}
	close(fd);
	process_exit(0);

	return 0;
}
