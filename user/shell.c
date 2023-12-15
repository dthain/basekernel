#include "library/string.h"
#include "library/syscalls.h"
#include "kernel/types.h"
#include "library/stdio.h"
#include "library/stdlib.h"
#include "kernel/ascii.h"
#include "library/errno.h"
#include "library/kernel_object_string.h"

#define MAX_LINE_LENGTH 1024

void print_directory(char *d, int length)
{
	while(length > 0) {
		printf("%s\n", d);
		int len = strlen(d) + 1;
		d += len;
		length -= len;
	}
}

void do_table()
{
	printf("Object Table:\n");
	char tag[16];
	int i, max = syscall_object_max();
	for(i=0;i<=max;i++) {
		int type = syscall_object_type(i);
		if(type>=0) {
			tag[0] = 0;
			syscall_object_get_tag(i,tag,sizeof(tag));
			printf("%d: %s (%s)\n",i,kernel_object_string(type),tag);
		}
	}
	printf("\n");
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
			int fd = syscall_open_file(KNO_STDDIR,argv[0],0,0);
			if(fd>=0) {
				int pid = syscall_process_fork();
				if(pid != 0) {
					printf("started process %d\n", pid);
				} else {
					syscall_process_exec(fd, 2, argv);
				}
			} else {
				printf("couldn't find %s: %s\n",argv[0],strerror(fd));
			}

		} else {
			printf("start: missing argument\n");
		}
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
			int fd = syscall_open_file(KNO_STDDIR,argv[0],0,0);
			if(fd>=0) {
				int pid = syscall_process_run(fd, i,  &argv[0]);
				if(pid > 0) {
					printf("started process %d\n", pid);
					syscall_process_yield();
					struct process_info info;
					syscall_process_wait(&info, -1);
					printf("process %d exited with status %d\n", info.pid, info.exitcode);
					syscall_process_reap(info.pid);
				} else {
					printf("couldn't run %s: %s\n", argv[0],strerror(pid));
				}
				syscall_object_close(fd);
			} else {
				printf("couldn't find %s: %s\n", argv[0],strerror(fd));
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
		const char *arg = strtok(0," ");
		if(!arg) arg = "/";
		char buffer[1024];
		int fd = syscall_open_dir(KNO_STDDIR,arg,0);
		if(fd>=0) {
			int length = syscall_object_list(fd, buffer, 1024);
			syscall_object_close(fd);
			print_directory(buffer, length);
		}
	} else if(pch && !strcmp(pch, "enter")) {
		char *path = strtok(0, " ");
		if(!path) {
			printf("Incorrect arguments, usage: enter <path>\n");
			return 1;
		}
		int fd = syscall_open_dir(KNO_STDDIR,path,0);
		if(fd>=0) {
			syscall_object_copy(fd,KNO_STDDIR);
			syscall_object_close(fd);
			printf("entered %s\n",path);
		} else {
			printf("couldn't enter %s: %s\n",path,strerror(fd));
		}
	} else if(pch && !strcmp(pch,"table")) {
		do_table();
	} else if(pch && !strcmp(pch, "help")) {
		printf("Commands:\necho <text>\nrun <path>\nmount <unit_no> <fs_type>\nlist\nstart <path>\nkill <pid>\nreap <pid>\nwait\ntable\nhelp\nexit\n");
	} else if(pch && !strcmp(pch, "exit")) {
		exit(0);
	} else if(pch) {
		printf("%s: command not found\n", pch);
	}
	return 0;
}

int readline( char *line, int length )
{
	int i = 0;
	char c;
	while(1) {
		syscall_object_read(0, &c, 1, 0);
		if(c == ASCII_CR) {
			printf_putchar(c);
			flush();
			line[i] = 0;
			return i;
		} else if(c == ASCII_BS) {
		       	if(i>0) {
				i--;
				printf_putchar(c);
				flush();
			}
		} else {
			if(i<(length-1)) {
				line[i] = c;
				i++;
				printf_putchar(c);
				flush();
			}
		}
	}
}

int main(int argc, char *argv[])
{
	char line[MAX_LINE_LENGTH];

	do_table();

	while(1) {
		printf("shell> ");
		flush();
		if(readline(line,sizeof(line))) {
			do_command(line);
		}
	}
}
