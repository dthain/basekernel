#include "library/string.h"
#include "library/syscalls.h"
#include "library/user-io.h"


int main() {


	uint32_t t[4];
	int t2 = get_window_properties(3, t);


	if (t2 < 0) {
		printf("Didnt work\n");
		return 1;
	}

	printf("X: %d\n", t[0]);
	printf("Y: %d\n", t[1]);
	printf("W: %d\n", t[2]);
	printf("H: %d\n", t[3]);


	int wd = open_window(3, 500, 500, 524, 200);
	t2 = get_window_properties(wd, t);

	if (t2 < 0) {
		printf("Didnt work\n");
		return 1;
	}

	printf("X: %d\n", t[0]);
	printf("Y: %d\n", t[1]);
	printf("W: %d\n", t[2]);
	printf("H: %d\n", t[3]);

	// printf("%d\n", wd);
	// process_fork();
	// const char *args[] = { "snake.exe" };
	// process_exec("snake.exe", args, 1);

	return 0;
}