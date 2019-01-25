/*
Copyright (C) 2016-2019 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/

/*
This module is the runtime start of every user-level program.
The very first symbol in this module must be _start() because
the kernel simply jumps to the very first location of the executable.
_start() sets up any necessary runtime environment and invokes
the main function.  Note that this function cannot exit, but
must invoke the syscall_process_exit() system call to terminate the process.
*/

#include "library/syscalls.h"

int main(int argc, const char *argv[]);

void _start(int argc, const char **argv)
{
	syscall_process_exit(main(argc, argv));
}
