/*
Copyright (C) 2018 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/

#include "library/syscalls.h"
#include "kernel/syscall.h"
#include "library/string.h"

int main(int argc, char const *argv[]) {
	if (argc <= 1) {
		printf("No program to run\n");
		return 1;
	}
	unsigned int startTime = gettimeofday();
	int pid = process_fork();
	if (pid == 0) { // child
		process_exec(argv[1], &argv[1], argc-1);
		printf("exec failed\n");
		return 1;
	} else if (pid < 0) {
		printf("fork failed\n");
		return -1;
	}
	/* parent */
	struct process_info info;
	process_wait(&info, -1);
	unsigned int timeElapsed = gettimeofday() - startTime;
	printf("Process %u exited with status %d\n", info.pid, info.exitcode);
	printf("Time elapsed: %d:%d:%d\n", timeElapsed/3600, (timeElapsed%3600)/60, timeElapsed % 60);
	printf("Disk 0: %d blocks read, %d blocks written\n", info.stat.blocks_read[0], info.stat.blocks_written[0]);
	printf("Disk 1: %d blocks read, %d blocks written\n", info.stat.blocks_read[1], info.stat.blocks_written[1]);
	printf("Disk 2: %d blocks read, %d blocks written\n", info.stat.blocks_read[2], info.stat.blocks_written[2]);
	printf("Disk 3: %d blocks read, %d blocks written\n", info.stat.blocks_read[3], info.stat.blocks_written[3]);

	printf("System calls used:\n");
	for (int i = 0; i < MAX_SYSCALL; i++) {
		if (info.stat.syscall_count[i]) {
			printf("Syscall %d: %u\n", i, info.stat.syscall_count[i]);
		}
	}



	return 0;
}
