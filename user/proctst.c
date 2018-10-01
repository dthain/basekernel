#include "library/string.h"
#include "library/syscalls.h"
#include "library/user-io.h"

int main() {
	int child_1 = process_fork();

	if (child_1 == 0) {
		printf("Child 1 waiting for awhile\n");
		process_sleep(10000);
	} else {
		printf("None\n");
	}


	int child_2 = process_fork();

	if (child_2 == 0) {
		printf("In child_2 shouldn't wait\n");
	} else {

	}
	printf("Child 2 returned\n");

	struct process_info info;
	info.pid = child_1;
	process_wait(&info, -1);

	printf("Child 1 returned\n");

	return 0;
}