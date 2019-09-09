/*
Copyright (C) 2016-2019 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/

#include "library/syscalls.h"
#include "library/string.h"

int main(int argc, char *argv[])
{
	struct kernel_io_stats s = {0};
	if (syscall_system_stats(&s)) {
		return 1;
	}

	printf("System uptime: %u:%u:%u\n", s.time / (3600), (s.time % 3600) / 60, s.time % 60);
	printf("%u reads %u bytes %u writes %u bytes",
	       s.read_ops, s.read_bytes, s.write_ops, s.write_bytes );

	return 0;
}
