/*
 *
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
	int n;
	char buffer[1000];
	mount(0, "kevin", "K");
	mount(2, "cdrom", "CD");
  ns_change("CD");
	chdir("/");
	int fd1 = open("TEST.EXE", 1, 0);
	printf("got fd %d for cdrom\n", fd1);
  ns_change("K");
	chdir("/");
	int fd2 = open("testcopy", 2, 0);
	printf("got fd %d for kevinfs\n", fd2);
	printf("copying...\n");
	while((n = read(fd1, buffer, 1000)) > 0) {
		write(fd2, buffer, n);
	}
	printf("done!\n");
	close(fd1);
	close(fd2);
	exit(0);
  
	return 0;
}
