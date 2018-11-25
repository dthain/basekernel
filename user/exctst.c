#include "library/syscalls.h"
#include "library/string.h"

int main()
{
	int pid = process_fork();

	if (pid == 0) {
		printf("hello world, I am the child %d.\n", process_self());
		const char *args[] = { "snake.exe" };
		process_exec("snake.exe", args, 1);
	} else {
		printf("hello world, I am the parent %d.\n", process_self());
		struct process_info info;
		process_wait(&info, -1);
		process_reap(info.pid);
	}

	return 0;
}
