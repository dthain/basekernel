#include "library/string.h"
#include "library/syscalls.h"
#include "library/user-io.h"


int main() {
	/* Test get_dimensions for window syscall */
	int t1[4];
	int wd = 3;

	if (get_dimensions(wd, t1) < 0) {
		printf("Didnt work\n");
		return 1;
	}

	printf("X: %d\n", t1[0]);
	printf("Y: %d\n", t1[1]);
	printf("W: %d\n", t1[2]);
	printf("H: %d\n", t1[3]);

	/* Test get dimensions for a file */
	int t2[2];
	const char * path = "./bin/snake.exe";
	int fd = open(path, 0, 0);

	if (get_dimensions(fd, t2) < 0) {
		printf("Didnt work\n");
		return 1;
	}

	printf("Size:     %d\n", t2[0]);
	printf("Refcount: %d\n", t2[1]);

	return 0;
}