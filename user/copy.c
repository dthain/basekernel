/*
Copyright (C) 2016-2019 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/

/* duplicates cdrom to kevinfs disk */

#include "library/syscalls.h"
#include "library/string.h"
#include "library/errno.h"

int main(int argc, char *argv[])
{
	if(argc!=3) {
		printf("%s: <sourcepath> <destpath>\n");
		return 1;
	}

	int src = syscall_open_file(argv[1],0,0);
	if(src<0) {
		printf("couldn't open %s: %s\n",argv[1],strerror(src));
		return 1;
	}

	int dst = syscall_open_file(argv[2],0,0);
	if(dst<0) {
		printf("couldn't open %s: %s\n",argv[2],strerror(dst));
		return 1;
	}

	printf("copying %s to %s...\n",argv[1],argv[2]);
	int result = syscall_object_copy(src,dst);
	if(result<0) {
		printf("copy failed: %s\n",strerror(result));
		return 1;
	}

	printf("copy complete\n");
	return 0;
}
