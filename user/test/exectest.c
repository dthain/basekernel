#include "library/syscalls.h"
#include "library/string.h"

int main(int argc, char *argv[])
{
	int pid = syscall_process_fork();

	if (pid == 0) {
		printf("hello world, I am the child %d.\n", syscall_process_self());
		const char *args[] = { "snake.exe" };
		syscall_process_exec("snake.exe", 1, args);
	} else {
		printf("hello world, I am the parent %d.\n", syscall_process_self());
		struct process_info info;
		syscall_process_wait(&info, -1);
		syscall_process_reap(info.pid);
	}

	return 0;
}
