/*
Copyright (C) 2017 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/


#include "library/syscalls.h"
#include "library/user-io.h"
#include "library/string.h"

int main(const char *argv[], int argc)
{

	const char *args[] = { "snake.exe" };
	int pid = process_run("snake.exe", args, 1);
	if(pid > 0) {
		printf("Started Process\n");
		process_yield();
		struct process_info info;
		process_wait(&info, -1);
		process_reap(info.pid);
	} else {
		printf("couldn't start %s\n", args[0]);
	}
	return 0;
}
