#include "kshell.h"
#include "keyboard.h"
#include "console.h"
#include "string.h"
#include "rtc.h"
#include "syscall.h"
#include "cdromfs.h"
#include "kmalloc.h"
#include "process.h"
#include "main.h"
#include "ascii.h"
#include "fs.h"

static int print_directory( char *d, int length )
{
	while(length>0) {
		console_printf("%s\n",d);
		int len = strlen(d)+1;
		d += len;
		length -= len;
	}
	return 0;
}

static int mount_cd( int unit )
{
	struct fs *cdrom = fs_get("cdrom");
	struct volume *v = fs_mount(cdrom, unit);
	if(v) {
		struct dirent *d = fs_root(v);
		if(d) {
            root_directory = d;
            return 0;
		} else {
			printf("couldn't access root dir!\n");
            return 1;
		}
		fs_umount(v);
	} else {
		printf("couldn't mount filesystem!\n");
        return 2;
	}

	return 3;
}

static int list_directory( const char *path )
{
    struct dirent *d = root_directory;
    if(d) {
        int buffer_length = 1024;
        char *buffer = kmalloc(buffer_length);
        if(buffer) {
            int length = fs_readdir(d,buffer,buffer_length);
            print_directory(buffer,length);
            kfree(buffer);
        }
    } else {
        printf("couldn't access root dir!\n");
    }

	return 0;
}


static int process_command(char *line)
{
	const char *pch = strtok(line, " ");
	if (pch && !strcmp(pch, "echo"))
	{
		pch = strtok(0, " ");
		if (pch) printf("%s\n", pch);
	}
	else if (pch && !strcmp(pch, "start"))
	{
		pch = strtok(0, " ");
		if (pch) {
			sys_run(pch, pch, "start", 0);
			process_yield();
		}
		else
			list_directory("run: missing argument");
	}
	else if (pch && !strcmp(pch, "run"))
	{
		pch = strtok(0, " ");
		if (pch) {
			sys_run(pch, pch, "run", 0);
            struct process_info info;
            if (process_wait_child(&info, 5000)) {
                printf("process %d exited with status %d\n", info.pid, info.exitcode);

            } else {
                printf("run: timeout\n");
            }
		}
		else
			list_directory("run: missing argument");
	}
	else if (pch && !strcmp(pch, "mount"))
	{
		pch = strtok(0, " ");
        int unit;
		if (pch && str2int(pch, &unit)) {
		    mount_cd(unit);	
        }
		else
			printf("mount: expected unit number but got %s\n", pch);

	}
	else if (pch && !strcmp(pch, "kill"))
	{
		pch = strtok(0, " ");
        int pid;
		if (pch && str2int(pch, &pid)) {
		    process_kill(pid);	
        }
		else
			printf("kill: expected process id number but got %s\n", pch);

	}
	else if (pch && !strcmp(pch, "wait"))
	{
		pch = strtok(0, " ");
		if (pch)
			printf("%s: unexpected argument\n", pch);
		else {
            struct process_info info;
            if (process_wait_child(&info, 5000)) {
                printf("process %d exited with status %d\n", info.pid, info.exitcode);

            } else {
                printf("wait: timeout\n");
            }
        }

	}
	else if (pch && !strcmp(pch, "list"))
	{
		pch = strtok(0, " ");
		if (pch)
			printf("%s: unexpected argument\n", pch);
		else
			list_directory("/");

	}
	else if (pch && !strcmp(pch, "stress"))
	{
		pch = strtok(0, " ");
		if (pch)
			printf("%s: unexpected argument\n", pch);
		else {
            int i;
            for (i = 0; i < 100; i++) {
                sys_run("TEST.EXE");
                clock_wait(1000);
            }
        }

	}
	else if (pch && !strcmp(pch, "test"))
	{
		pch = strtok(0, " ");
		if (pch && !strcmp(pch, "kmalloc"))
			kmalloc_test();
		else if (pch)
			printf("test: test '%s' not found\n", pch);
		else
			printf("test: missing argument\n");
	}
	else if (pch && !strcmp(pch, "time"))
	{
		pch = strtok(0, " ");
		if (pch)
			printf("%s: unexpected argument\n", pch);
		else
		{
			struct rtc_time time;
			rtc_read(&time);
			printf(
				"%d-%d-%d %d:%d:%d\n",
				time.year,
				time.month,
				time.day,
				time.hour,
				time.minute,
				time.second
			);
		}

	}
	else if (pch && !strcmp(pch, "help"))
	{
		printf(
			"%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n",
			"Commands:",
			"echo <text>",
			"run <path>",
			"start <path>",
            "kill <pid>",
            "wait",
			"test <function>",
			"list",
			"time",
			"help",
			"exit",
            "stress"
		);
	}
	else if (pch && !strcmp(pch, "exit"))
	{
		return -1;
	}
	else if (pch)
	{
		printf("%s: command not found\n", pch);
	}
	return 0;
}

int kshell_launch()
{
	char line[1024];
	char *pos = line;
	printf ("$ ");
	while(1)
	{
		char c = keyboard_read();
		if (pos == line && c == ASCII_BS)
			continue;
		console_putchar(c);
		if (c == ASCII_CR)
		{
			int res = process_command(line);
			if (res < 0)
				break;
			pos = line;
			printf("$ ");
		}
		else if (c == ASCII_BS)
		{
			pos--;
		}
		else
		{
			*pos = c;
			pos++;
		}
		*pos = 0;
	}
	return 0;
}
