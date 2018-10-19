#include "library/string.h"
#include "library/syscalls.h"
#include "library/user-io.h"


int main() {
	/* Test get_dimensions syscall */
	int t[4];
	int wd = 3; // open_window(3, 500, 500, 524, 200);
	// int t2 = get_dimensions(wd, t);
	draw_window(wd);
	draw_string(600, 600, "Press any key to start");
	draw_flush();


	process_sleep(10000);

	// if (t2 < 0) {
	// 	printf("Didnt work\n");
	// 	return 1;
	// }

	// printf("X: %d\n", t[0]);
	// printf("Y: %d\n", t[1]);
	// printf("W: %d\n", t[2]);
	// printf("H: %d\n", t[3]);

	return 0;
}