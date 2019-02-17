#include "library/syscalls.h"
#include "library/user-io.h"
#include "library/string.h"

int main(int argc, char *argv[])
{
	printf("%d: Running pipe test!\n", syscall_process_self());
	int w = syscall_open_pipe();
	syscall_object_set_blocking(w, 0);
	int x = syscall_process_fork();
	if(x) {
		printf("%d: Writing...\n", syscall_process_self());
		syscall_object_dup(w, KNO_STDOUT);
		printf("Testing!\n");
		syscall_process_sleep(1000);
	} else {
		printf("%d: Reading...\n", syscall_process_self());
		int r;
		char buf[10];
		while(!(r = syscall_object_read(w, buf, 10))) {
			syscall_process_yield();
		}
		printf("%d: I read %d chars from my brother\n", syscall_process_self(), r);
		printf("%d: They are (%s)\n", syscall_process_self(), buf);
	}
	return 0;
}
