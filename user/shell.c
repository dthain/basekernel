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

int process_command(char *line)
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
			int pid = process_fork();
			if(pid != 0) {
				printf("started process %d\n", pid);
			} else {
				process_exec(pch, argv, 2);
			}
		} else
			printf("start: missing argument\n");
	} else if(pch && !strcmp(pch, "run")) {
		pch = strtok(0, " ");
		// if(pch) {
		// 	const char *argv[20];
		// 	argv[0] = pch;
		// 	int i = 1;
		// 	char *next;
		// 	while((next = strtok(0, " "))) {
		// 		argv[i++] = next;
		// 	}
		// 	int pid = process_fork();
		// 	if(pid != 0) {
		// 		printf("started process %d\n", pid);
		// 		struct process_info info;
		// 		process_wait(&info, -1);
		// 		printf("process %d exited with status %d\n", info.pid, info.exitcode);
		// 		process_reap(info.pid);
		// 	} else {
		// 		process_exec(pch, argv, i);
		// 	}
		// } else
		// 	printf("run: missing argument\n");

		if(pch) {
			const char *argv[20];
			argv[0] = pch;
			int i = 1;
			char *next;
			while((next = strtok(0, " "))) {
				argv[i++] = next;
			}
			int pid = process_run(argv[0], &argv[0], i);
			if(pid > 0) {
				printf("started process %d\n", pid);
				process_yield();
				struct process_info info;
				process_wait(&info, -1);
				printf("process %d exited with status %d\n", info.pid, info.exitcode);
				process_reap(info.pid);
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
			if(process_reap(pid)) {
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
			process_kill(pid);
		} else
			printf("kill: expected process id number but got %s\n", pch);

	} else if(pch && !strcmp(pch, "wait")) {
		pch = strtok(0, " ");
		if(pch)
			printf("%s: unexpected argument\n", pch);
		else {
			struct process_info info;
			if(process_wait(&info, 5000) > 0) {
				printf("process %d exited with status %d\n", info.pid, info.exitcode);
			} else {
				printf("wait: timeout\n");
			}
		}
	} else if(pch && !strcmp(pch, "list")) {
		char buffer[1024];
		int length = readdir(".", buffer, 1024);
		print_directory(buffer, length);
	} else if(pch && !strcmp(pch, "chdir")) {
		char *path = strtok(0, " ");
		if(!path) {
			printf("Incorrect arguments, usage: chdir <path>\n");
			return 1;
		}
		chdir(path);
	} else if(pch && !strcmp(pch, "help")) {
		printf("%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n", "Commands:", "echo <text>", "run <path>", "mount <unit_no> <fs_type>", "list", "start <path>", "kill <pid>", "reap <pid>", "wait", "help", "exit");
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
		read(0, &c, 1);
		if(pos == line && c == ASCII_BS)
			continue;
		printf_putchar(c);
		flush();
		if(c == ASCII_CR) {
			int res = process_command(line);
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
