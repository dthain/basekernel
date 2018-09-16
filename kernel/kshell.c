
#include "kernel/types.h"
#include "kernel/error.h"
#include "kernel/ascii.h"
#include "kshell.h"
#include "keyboard.h"
#include "console.h"
#include "string.h"
#include "rtc.h"
#include "kmalloc.h"
#include "memory.h"
#include "process.h"
#include "main.h"
#include "fs.h"
#include "syscall_handler.h"
#include "clock.h"
#include "kernelcore.h"

static int kshell_mount(int unit, const char *fs_type)
{
	struct fs *fs = fs_lookup(fs_type);
	if(!fs) {
		printf("invalid fs type: %s\n", fs_type);
		return -1;
	}
	struct fs_volume *v = fs_volume_open(fs, unit);
	if(v) {
		struct fs_dirent *d = fs_volume_root(v);
		if(d) {
			current->root_dir = d;
			current->current_dir =  fs_dirent_addref(d);
			return 0;
		} else {
			printf("couldn't access root dir!\n");
			return 1;
		}
		fs_volume_close(v);
	} else {
		printf("couldn't mount filesystem!\n");
		return 2;
	}

	return 3;
}

static int kshell_printdir( const char *d, int length)
{
	while(length > 0) {
		console_printf("%s\n", d);
		int len = strlen(d) + 1;
		d += len;
		length -= len;
	}
	return 0;
}

static int kshell_listdir(const char *path)
{
	struct fs_dirent *d = fs_resolve(path);
	if(d) {
		int buffer_length = 1024;
		char *buffer = kmalloc(buffer_length);
		if(buffer) {
			int length = fs_dirent_readdir(d, buffer, buffer_length);
			if(length>0) {
				kshell_printdir(buffer, length);
			} else {
				printf("list: %s is not a directory\n",path);
			}
			kfree(buffer);
		}
	} else {
		printf("list: %s does not exist\n",path);
	}

	return 0;
}

static int kshell_execute( const char **argv, int argc )
{
	const char *cmd = argv[0];

	if(!strcmp(cmd,"start")) {
		if(argc>1) {
			int pid = sys_process_run(argv[1],&argv[1],argc-1);
			if(pid>0) {
				printf("started process %d\n", pid);
				process_yield();
			} else {
				printf("couldn't start %s\n",argv[1]);
			}
		} else {
			printf("run: requires argument.\n");
		}
	} else if(!strcmp(cmd,"exec")) {
		if(argc>1) {
			sys_process_exec(argv[1],&argv[1],argc-1);
			process_yield();
			printf("couldn't exec %s\n",argv[1]);
		} else {
			printf("exec: requires argument.\n");
		}
	} else if(!strcmp(cmd,"run")) {
		if(argc>1) {
			int pid = sys_process_run(argv[1],&argv[1],argc-1);
			if(pid>0) {
				printf("started process %d\n", pid);
				process_yield();
				struct process_info info;
				process_wait_child(pid,&info, -1);
				printf("process %d exited with status %d\n", info.pid, info.exitcode);
				process_reap(info.pid);
			} else {
				printf("couldn't start %s\n",argv[1]);
			}
		} else {
			printf("run: requires argument\n");
		}
	} else if(!strcmp(cmd,"mount")) {
		if(argc>1) {
			int unit;
			if(str2int(argv[1],&unit)) {
				kshell_mount(unit, argv[2]);
			} else {
				printf("mount: expected unit number but got %s\n",argv[1]);
			}
		} else {
			printf("mount: requires argument\n");
		}
	} else if(!strcmp(cmd,"reap")) {
		if(argc>1) {
			int pid;
			if(str2int(argv[1],&pid)) {
				if(process_reap(pid)) {
					printf("reap failed!\n");
				} else {
					printf("process %d reaped\n",pid);
				}
			} else {
				printf("reap: expected process id but got %s\n", argv[1]);	
			}
		} else {
			printf("reap: requires argument\n");
		}
	} else if(!strcmp(cmd,"kill")) {
		if(argc>1) {
			int pid;
			if(str2int(argv[1],&pid)) {
				process_kill(pid);
			} else {
				printf("kill: expected process id number but got %s\n", argv[1]);
			}
		} else {
			printf("kill: requires argument\n");
		}

	} else if(!strcmp(cmd,"wait")) {
		struct process_info info;
		if(process_wait_child(0,&info,5000)>0) {
			printf("process %d exited with status %d\n", info.pid, info.exitcode);
		} else {
			printf("wait: timeout\n");
		}
	} else if(!strcmp(cmd,"list")) {
		if(argc>1) {
			kshell_listdir(argv[1]);
		} else {
			kshell_listdir(".");
		}

	} else if(!strcmp(cmd,"stress")) {
		while(1) {
			const char *argv[] = { "test.exe", "arg1", "arg2", "arg3", "arg4", "arg5", 0 };
			int pid = sys_process_run("/bin/test.exe", argv, 6);
			if(pid>0) {
				struct process_info info;
				process_wait_child(pid,&info,-1);
				printf("process %d exited with status %d\n", info.pid, info.exitcode);
				process_reap(pid);
			} else {
				printf("run failed\n");
				clock_wait(1000);
			}
			printf("memory: %d/%d\n",memory_pages_free(),memory_pages_total());
		}
	} else if(!strcmp(cmd,"mkdir")) {
		if(argc==2) {
			sys_mkdir(argv[1]);
		} else {
			printf("mkdir: missing argument\n");
		}
	} else if(!strcmp(cmd,"format")) {
		if(argc==3) {
			int unit;
			if(str2int(argv[1],&unit)) {
				struct fs *f = fs_lookup(argv[2]);
			
				if(!f) {
					printf("invalid fs type: %s\n",argv[2]);
				} else {
					fs_mkfs(f, unit);
				}
			} else {
				printf("mount: expected unit number but got %s\n", argv[1]);
			}
		}
	} else if(!strcmp(cmd,"rmdir")) {
		if(argc==2) {
			sys_rmdir(argv[1]);
		} else {
			printf("rmdir: missing argument\n");
		}
	} else if(!strcmp(cmd,"chdir")) {
		if(argc==2) {
			sys_chdir(argv[1]);
		} else {
			printf("chdir: missing argument\n");
		}
	} else if(!strcmp(cmd,"time")) {
		struct rtc_time time;
		rtc_read(&time);
		printf("%d-%d-%d %d:%d:%d\n", time.year, time.month, time.day, time.hour, time.minute, time.second);
	} else if(!strcmp(cmd,"reboot")) {
		reboot();
	} else if(!strcmp(cmd,"help")) {
		  printf("Kernel Shell Commands:\nrun <path> <args>\nstart <path> <args>\nkill <pid>\nreap <pid>\nwait\nlist\nmount <device> <fstype>\nformat <device> <fstype\nchdir <path>\nmkdir <path>\nrmdir <path>time\n\nreboot\nhelp\n\n");
	} else {
		printf("%s: command not found\n", argv[0]);
	}
	return 0;
}

int kshell_readline( char *line, int length )
{
	int i = 0;
	while(i<(length-1)) {
		char c = keyboard_read(0);
		if(c == ASCII_CR) {
			line[i] = 0;
			printf("\n");
			return 1;
		} else if(c == ASCII_BS) {
			if(i>0) {
				console_putchar(c);
				i--;
			}
		} else if(c >= 0x20 && c <= 0x7E) {
			console_putchar(c);
			line[i] = c;
			i++;
		}
	}

	return 0;
}


int kshell_launch()
{
	char line[1024];
	const char *argv[100];
	int argc;

	while(1) {
		printf("kshell> ");
		kshell_readline(line,sizeof(line));

		argc = 0;
		argv[argc] = strtok(line," ");
		while(argv[argc]) {
			argc++;
			argv[argc] = strtok(0," ");
		}

		if(argc>0) {
			kshell_execute(argv,argc);
		}
	}

	return 0;
}
