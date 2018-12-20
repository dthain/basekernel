#include "library/string.h"
#include "library/syscalls.h"
#include "kernel/types.h"
#include "library/user-io.h"
#include "kernel/ascii.h"

void print_directory(char *d, int length)
{
	while(length > 0) {
		printf("%s\n", d);
		int len = strlen(d) + 1;
		d += len;
		length -= len;
	}
}

int do_command(char *line)
{
	const char *pch = strtok(line, " ");
	if(pch && !strcmp(pch, "echo")) {
		pch = strtok(0, " ");
		if(pch)
			printf("%s\n", pch);
	} else if(pch && !strcmp(pch, "start")) {
		pch = strtok(0, " ");
		if(pch) {
			const char *argv[20];
			argv[0] = pch;
			int i = 1;
			char *next;
			while((next = strtok(0, " "))) {
				argv[i++] = next;
			}
			int pid = syscall_process_fork();
			if(pid != 0) {
				printf("started process %d\n", pid);
			} else {
				syscall_process_exec(pch, argv, 2);
			}
		} else
			printf("start: missing argument\n");
	} else if(pch && !strcmp(pch, "run")) {
		pch = strtok(0, " ");
		if(pch) {
			const char *argv[20];
			argv[0] = pch;
			int i = 1;
			char *next;
			while((next = strtok(0, " "))) {
				argv[i++] = next;
			}
			int pid = syscall_process_run(argv[0], &argv[0], i);
			if(pid > 0) {
				printf("started process %d\n", pid);
				syscall_process_yield();
				struct process_info info;
				syscall_process_wait(&info, -1);
				printf("process %d exited with status %d\n", info.pid, info.exitcode);
				syscall_process_reap(info.pid);
			} else {
				printf("Couldn't start %s\n", argv[1]);
			}
		} else {
			printf("run: requires argument\n");
		}
	} else if(pch && !strcmp(pch, "reap")) {
		pch = strtok(0, " ");
		int pid;
		if(pch && str2int(pch, &pid)) {
			if(syscall_process_reap(pid)) {
				printf("reap failed!\n");
			} else {
				printf("processed reaped!\n");
			}
		} else
			printf("reap: expected process id number but got %s\n", pch);
	} else if(pch && !strcmp(pch, "kill")) {
		pch = strtok(0, " ");
		int pid;
		if(pch && str2int(pch, &pid)) {
			syscall_process_kill(pid);
		} else
			printf("kill: expected process id number but got %s\n", pch);

	} else if(pch && !strcmp(pch, "wait")) {
		pch = strtok(0, " ");
		if(pch)
			printf("%s: unexpected argument\n", pch);
		else {
			struct process_info info;
			if(syscall_process_wait(&info, 5000) > 0) {
				printf("process %d exited with status %d\n", info.pid, info.exitcode);
			} else {
				printf("wait: timeout\n");
			}
		}
	} else if(pch && !strcmp(pch, "list")) {
		char buffer[1024];
		int fd = syscall_open_file(".",0,0);
		if(fd>=0) {
			int length = syscall_object_readdir(fd, buffer, 1024);
			syscall_object_close(fd);
			print_directory(buffer, length);
		}
	} else if(pch && !strcmp(pch, "chdir")) {
		char *path = strtok(0, " ");
		if(!path) {
			printf("Incorrect arguments, usage: chdir <path>\n");
			return 1;
		}
		syscall_chdir(path);
	} else if(pch && !strcmp(pch, "help")) {
		printf("Commands:\necho <text>\nrun <path>\nmount <unit_no> <fs_type>\nlist\nstart <path>\nkill <pid>\nreap <pid>\nwait\nhelp\nexit\n");
	} else if(pch && !strcmp(pch, "exit")) {
		return -1;
	} else if(pch) {
		printf("%s: command not found\n", pch);
	}
	return 0;
}

int main(char **argv, int argc)
{
	printf("User shell ready:\n");
	char line[1024];
	char *pos = line;
	char c;
	printf("u$ ");
	while(1) {
		flush();
		syscall_object_read(0, &c, 1);
		if(pos == line && c == ASCII_BS)
			continue;
		printf_putchar(c);
		flush();
		if(c == ASCII_CR) {
			int res = do_command(line);
			if(res < 0) {
				break;
			}
			pos = line;
			printf("u$ ");
		} else if(c == ASCII_BS) {
			pos--;
		} else {
			*pos = c;
			pos++;
		}
		*pos = 0;
	}
	return 0;
}
