/*
Copyright (C) 2016-2019 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/

#include "library/syscalls.h"
#include "library/string.h"

int main(int argc, char *argv[])
{
	struct system_stats s = {0};
	if (syscall_system_stats(&s)) {
		return 1;
	}

	printf("System uptime: %u:%u:%u\n", s.time / (3600), (s.time % 3600) / 60, s.time % 60);
	printf("Disk 0: %d blocks read, %d blocks written\n", s.blocks_read[0], s.blocks_written[0]);
	printf("Disk 1: %d blocks read, %d blocks written\n", s.blocks_read[1], s.blocks_written[1]);
	printf("Disk 2: %d blocks read, %d blocks written\n", s.blocks_read[2], s.blocks_written[2]);
	printf("Disk 3: %d blocks read, %d blocks written\n", s.blocks_read[3], s.blocks_written[3]);


	return 0;
}
