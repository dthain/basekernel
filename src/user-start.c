/*
Copyright (C) 2017 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/

/*
This module is the runtime start of every user-level program.
The very first symbol in this module must be _start() because
the kernel simply jumps to the very first location of the executable.
_start() sets up any necessary runtime environment and invokes
the main function.  Note that this function cannot exit, but
must invoke the exit() system call to terminate the process.
*/

#include "syscalls.h"
#include "memorylayout.h"

int main( const char *argv[], int argc );

void _start(const char** argv, int argc) {
	exit(main(argv, argc));
}

