/*
Copyright (C) 2018 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/

/* duplicates cdrom to kevinfs disk */

#include "library/syscalls.h"
#include "library/string.h"

int main(int argc, char const *argv[]) {
	dup_volume(2,0,"cdrom","kevinfs");
	printf("syscall finished\n");
	return 0;
}
