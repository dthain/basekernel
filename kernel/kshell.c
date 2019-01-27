/*
Copyright (C) 2016-2019 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/

#include "kernel/types.h"
#include "kernel/error.h"
#include "kernel/ascii.h"
#include "kshell.h"
#include "keyboard.h"
#include "console.h"
#include "string.h"
#include "rtc.h"
#include "kmalloc.h"
#include "page.h"
#include "process.h"
#include "main.h"
#include "fs.h"
#include "syscall_handler.h"
#include "clock.h"
#include "kernelcore.h"
#include "bcache.h"
#include "printf.h"

static int kshell_mount( const char *devname, int unit, const char *fs_type)
{
	struct device *dev = device_open(devname,unit);
	if(dev) {
		struct fs *fs = fs_lookup(fs_type);
		if(fs) {
			struct fs_volume *v = fs_volume_open(fs,dev);
			if(v) {
				struct fs_dirent *d = fs_volume_root(v);
				if(d) {
					if(current->root_dir) fs_dirent_close(current->root_dir);
					current->root_dir = d;
					current->current_dir = fs_dirent_addref(d);
					return 0;
				} else {
					printf("mount: couldn't find root dir on %s unit %d!\n",device_name(dev),device_unit(dev));
					return -1;
				}
				fs_volume_close(v);
			} else {
				printf("mount: couldn't mount %s on %s unit %d\n",fs_type,device_name(dev),device_unit(dev));
				return -1;
			}
		} else {
			printf("mount: invalid fs type: %s\n", fs_type);
			return -1;
		}
		device_close(dev);
	} else {
		printf("mount: couldn't open device %s unit %d\n",devname,unit);
		return -1;
	}

	return -1;
}

/*
Install software from the cdrom volume unit src
to the disk volume dst by performing a recursive copy.
XXX This needs better error checking.
*/

int kshell_install( int src, int dst )
{
	struct fs *srcfs = fs_lookup("cdromfs");
	struct fs *dstfs = fs_lookup("diskfs");

	if(!srcfs || !dstfs) return KERROR_NOT_FOUND;

	struct device *srcdev = device_open("atapi",src);
	struct device *dstdev = device_open("ata",dst);

	if(!srcdev || !dstdev) return KERROR_NOT_FOUND;

	struct fs_volume *srcvolume = fs_volume_open(srcfs,srcdev);
	struct fs_volume *dstvolume = fs_volume_open(dstfs,dstdev);

	if(!srcvolume || !dstvolume) return KERROR_NOT_FOUND;

	struct fs_dirent *srcroot = fs_volume_root(srcvolume);
	struct fs_dirent *dstroot = fs_volume_root(dstvolume);

	printf("copying atapi unit %d to ata unit %d...\n",src,dst);

	fs_dirent_copy(srcroot, dstroot,0);

	fs_dirent_close(dstroot);
	fs_dirent_close(srcroot);

	fs_volume_close(srcvolume);
	fs_volume_close(dstvolume);

	device_close(srcdev);

	bcache_flush_device(dstdev);
	device_close(dstdev);

	return 0;
}

static int kshell_printdir(const char *d, int length)
{
	while(length > 0) {
		printf("%s\n", d);
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
			int length = fs_dirent_list(d, buffer, buffer_length);
			if(length>=0) {
				kshell_printdir(buffer, length);
			} else {
				printf("list: %s is not a directory\n", path);
			}
			kfree(buffer);
		}
	} else {
		printf("list: %s does not exist\n", path);
	}

	return 0;
}

static int kshell_execute(int argc, const char **argv)
{
	const char *cmd = argv[0];

	if(!strcmp(cmd, "start")) {
		if(argc > 1) {
			int pid = sys_process_run(argv[1], argc - 1,  &argv[1]);
			if(pid > 0) {
				printf("started process %d\n", pid);
				process_yield();
			} else {
				printf("couldn't start %s\n", argv[1]);
			}
		} else {
			printf("run: requires argument.\n");
		}
	} else if(!strcmp(cmd, "exec")) {
		if(argc > 1) {
			sys_process_exec(argv[1], argc - 1, &argv[1]);
			process_yield();
			printf("couldn't exec %s\n", argv[1]);
		} else {
			printf("exec: requires argument.\n");
		}
	} else if(!strcmp(cmd, "run")) {
		if(argc > 1) {
			int pid = sys_process_run(argv[1], argc - 1, &argv[1]);
			if(pid > 0) {
				printf("started process %d\n", pid);
				process_yield();
				struct process_info info;
				process_wait_child(pid, &info, -1);
				printf("process %d exited with status %d\n", info.pid, info.exitcode);
				process_reap(info.pid);
			} else {
				printf("couldn't start %s\n", argv[1]);
			}
		} else {
			printf("run: requires argument\n");
		}
	} else if(!strcmp(cmd, "mount")) {
		if(argc==4) {
			int unit;
			if(str2int(argv[2], &unit)) {
				kshell_mount(argv[1],unit,argv[3]);
			} else {
				printf("mount: expected unit number but got %s\n", argv[2]);
			}
		} else {
			printf("mount: requires device, unit, and fs type\n");
		}
	} else if(!strcmp(cmd, "umount")) {
		if(current->root_dir) {
			printf("unmounting root directory\n");
			fs_dirent_close(current->root_dir);
			current->root_dir = 0;
		} else {
			printf("nothing currently mounted\n");
		}
	} else if(!strcmp(cmd, "reap")) {
		if(argc > 1) {
			int pid;
			if(str2int(argv[1], &pid)) {
				if(process_reap(pid)) {
					printf("reap failed!\n");
				} else {
					printf("process %d reaped\n", pid);
				}
			} else {
				printf("reap: expected process id but got %s\n", argv[1]);
			}
		} else {
			printf("reap: requires argument\n");
		}
	} else if(!strcmp(cmd, "kill")) {
		if(argc > 1) {
			int pid;
			if(str2int(argv[1], &pid)) {
				process_kill(pid);
			} else {
				printf("kill: expected process id number but got %s\n", argv[1]);
			}
		} else {
			printf("kill: requires argument\n");
		}

	} else if(!strcmp(cmd, "wait")) {
		struct process_info info;
		if(process_wait_child(0, &info, 5000) > 0) {
			printf("process %d exited with status %d\n", info.pid, info.exitcode);
		} else {
			printf("wait: timeout\n");
		}
	} else if(!strcmp(cmd, "list")) {
		if(argc > 1) {
			kshell_listdir(argv[1]);
		} else {
			kshell_listdir(".");
		}

	} else if(!strcmp(cmd, "stress")) {
		while(1) {
			const char *argv[] = { "test.exe", "arg1", "arg2", "arg3", "arg4", "arg5", 0 };
			int pid = sys_process_run("/bin/test.exe", 6, argv);
			if(pid > 0) {
				struct process_info info;
				process_wait_child(pid, &info, -1);
				printf("process %d exited with status %d\n", info.pid, info.exitcode);
				process_reap(pid);
			} else {
				printf("run failed\n");
				clock_wait(1000);
			}
			uint32_t nfree, ntotal;
			page_stats(&nfree,&ntotal);
			printf("memory: %d/%d\n",nfree,ntotal);
		}
	} else if(!strcmp(cmd, "mkdir")) {
		if(argc == 3) {
			struct fs_dirent *dir = fs_resolve(argv[1]);
			if(dir) {
				struct fs_dirent *n = fs_dirent_mkdir(dir,argv[2]);
				if(!n) {
					printf("mkdir: couldn't create %s\n",argv[2]);
				}
				fs_dirent_close(n);
				fs_dirent_close(dir);
			} else {
				printf("mkdir: couldn't open %s\n",argv[1]);
			}
		} else {
			printf("use: mkdir <parent-dir> <dirname>\n");
		}
	} else if(!strcmp(cmd, "format")) {
		if(argc == 4) {
			int unit;
			if(str2int(argv[2], &unit)) {
				struct fs *f = fs_lookup(argv[3]);
				if(f) {
					struct device *d = device_open(argv[1],unit);
					if(d) {
						fs_volume_format(f,d);
					} else {
						printf("couldn't open device %s unit %d\n",argv[1],unit);
					}
				} else {
					printf("invalid fs type: %s\n", argv[3]);
				}
			} else {
				printf("format: expected unit number but got %s\n", argv[2]);
			}
		}
	} else if(!strcmp(cmd,"install")) {
		if(argc==3) {
			int src, dst;
			str2int(argv[1], &src);
			str2int(argv[2], &dst);
			kshell_install(src,dst);
		} else {
			printf("install: expected unit #s for cdrom and disk\n");
		}

	} else if(!strcmp(cmd, "remove")) {
		if(argc == 3) {
			struct fs_dirent *dir = fs_resolve(argv[1]);
			if(dir) {
				int result = fs_dirent_remove(dir,argv[2]);
				if(result<0) {
					printf("remove: couldn't remove %s\n",argv[2]);
				}
				fs_dirent_close(dir);
			} else {
				printf("remove: couldn't open %s\n",argv[1]);
			}
		} else {
			printf("use: remove <parent-dir> <filename>\n");
		}
	} else if(!strcmp(cmd, "chdir")) {
		if(argc == 2) {
			sys_chdir(argv[1]);
		} else {
			printf("chdir: missing argument\n");
		}
	} else if(!strcmp(cmd, "time")) {
		struct rtc_time time;
		rtc_read(&time);
		printf("%d-%d-%d %d:%d:%d\n", time.year, time.month, time.day, time.hour, time.minute, time.second);
	} else if(!strcmp(cmd, "reboot")) {
		reboot();
	} else if(!strcmp(cmd, "bcache_stats")) {
		struct bcache_stats stats;
		bcache_get_stats(&stats);
		printf("%d rhit %d rmiss %d whit %d wmiss %d wback\n",
			stats.read_hits,stats.read_misses,
			stats.write_hits,stats.write_misses,
			stats.writebacks);
	} else if(!strcmp(cmd,"bcache_flush")) {
		bcache_flush_all();
	} else if(!strcmp(cmd, "help")) {
		printf("Kernel Shell Commands:\nrun <path> <args>\nstart <path> <args>\nkill <pid>\nreap <pid>\nwait\nlist\nmount <device> <unit> <fstype>\numount\nformat <device> <unit><fstype>\ninstall <srcunit> <dstunit>\nchdir <path>\nmkdir <path>\nremove <path>time\nbcache_stats\nbcache_flush\nreboot\nhelp\n\n");
	} else {
		printf("%s: command not found\n", argv[0]);
	}
	return 0;
}

int kshell_readline(char *line, int length)
{
	int i = 0;
	while(i < (length - 1)) {
		char c = keyboard_read(0);
		if(c == ASCII_CR) {
			line[i] = 0;
			printf("\n");
			return 1;
		} else if(c == ASCII_BS) {
			if(i > 0) {
				putchar(c);
				i--;
			}
		} else if(c >= 0x20 && c <= 0x7E) {
			putchar(c);
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
		kshell_readline(line, sizeof(line));

		argc = 0;
		argv[argc] = strtok(line, " ");
		while(argv[argc]) {
			argc++;
			argv[argc] = strtok(0, " ");
		}

		if(argc > 0) {
			kshell_execute(argc, argv);
		}
	}

	return 0;
}
