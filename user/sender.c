#include "library/syscalls.h"
#include "library/string.h"

int main(int argc, char *argv[])
{
	printf("%d: Running pipe test!\n", syscall_process_self());
	int w = syscall_open_pipe();
	int x = syscall_process_fork();
	if(x) {
		char buf[] = "Hello World\n";
		printf("%d: Writing...\n", syscall_process_self());

		syscall_object_write(w, buf, strlen(buf),KERNEL_IO_POST);
		printf("Testing!\n");
		syscall_process_sleep(1000);
	} else {
		printf("%d: Reading...\n", syscall_process_self());
		int r;
		char buf[20];
		while(!(r = syscall_object_read(w, buf, 20, KERNEL_IO_NONBLOCK))) {
			syscall_process_yield();
		}
		printf("%d: I read %d chars from my brother\n", syscall_process_self(), r);
		printf("%d: They are (%s)\n", syscall_process_self(), buf);
	}
	return 0;
}
