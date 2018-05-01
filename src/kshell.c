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
#include "kevinfs/kevinfs_test.h"
#include "syscall_handler.h"

#define HISTORY 10

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

static int mount_cd( int unit , char *fs_type )
{
	struct fs *fs = fs_get(fs_type);
	if (!fs) {
		printf("invalid fs type: %s\n", fs_type);
		return -1;
	}
	struct fs_volume *v = fs_volume_mount(fs, unit);
	if(v) {
		struct fs_dirent *d = fs_volume_root(v);
		if(d) {
            root_directory = d;
	    current_directory = d;
            return 0;
		} else {
			printf("couldn't access root dir!\n");
            return 1;
		}
		fs_volume_umount(v);
	} else {
		printf("couldn't mount filesystem!\n");
        return 2;
	}

	return 3;
}

static int list_directory( const char *path )
{
    struct fs_dirent *d = current_directory;
    if(d) {
        int buffer_length = 1024;
        char *buffer = kmalloc(buffer_length);
        if(buffer) {
            int length = fs_dirent_readdir(d,buffer,buffer_length);
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
            const char* argv[] = {pch, "start"};
			int pid = sys_process_run(pch, argv, 2);
            printf("started process %d\n", pid);
			process_yield();
		}
		else
			printf("start: missing argument\n");
	}
	else if (pch && !strcmp(pch, "run"))
	{
		pch = strtok(0, " ");
		if (pch) {
      const char* argv[] = {pch, "run"};
      int pid = sys_process_run(pch, argv, 2);
      printf("started process %d\n", pid);
      struct process_info info;
      process_wait_child(&info, -1);
      printf("process %d exited with status %d\n", info.pid, info.exitcode);
      process_reap(info.pid);
		}
		else
			printf("run: missing argument\n");
	}
	else if (pch && !strcmp(pch, "mount"))
	{
		pch = strtok(0, " ");
        int unit;
		if (pch && str2int(pch, &unit)) {
		    char *fs_type = strtok(0, " ");
		    mount_cd(unit, fs_type);
        }
		else
			printf("mount: expected unit number but got %s\n", pch);

	}
	else if (pch && !strcmp(pch, "reap"))
	{
		pch = strtok(0, " ");
        int pid;
		if (pch && str2int(pch, &pid)) {
		    if (process_reap(pid)) {
                printf("reap failed!\n");
            } else {
                printf("processed reaped!\n");
            }
        }
		else
			printf("reap: expected process id number but got %s\n", pch);

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
            if (!process_wait_child(&info, 5000)) {
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
            while (1) {
                const char *argv[] = {"TEXT.EXE","arg1","arg2","arg3","arg4","arg5",0};
                sys_process_run("TEST.EXE", argv, 6);
                struct process_info info;
                if (process_wait_child(&info, 5000)) {
                    printf("process %d exited with status %d\n", info.pid, info.exitcode);
                    if (info.exitcode != 0) return 1;
                }
            }
        }

	}
	else if (pch && !strcmp(pch, "test"))
	{
		pch = strtok(0, " ");
		if (pch && !strcmp(pch, "kmalloc"))
			kmalloc_test();
		else if (pch && !strcmp(pch, "kevinfs"))
			kevinfs_test();
		else if (pch)
			printf("test: test '%s' not found\n", pch);
		else
			printf("test: missing argument\n");
	}
	else if (pch && !strcmp(pch, "mkdir"))
	{
		pch = strtok(0, " ");
		if (pch)
			fs_dirent_mkdir(current_directory, pch);
		else
			printf("mkdir: missing argument\n");
	}
	else if (pch && !strcmp(pch, "format"))
	{
		pch = strtok(0, " ");
		int unit;
		if (pch && str2int(pch, &unit)) {
			char *fs_type = strtok(0, " ");
			struct fs *f = fs_get(fs_type);
			if (!f)
				printf("invalid fs type: %s\n", fs_type);
			else
				fs_mkfs(f, unit);
		}
		else
			printf("mount: expected unit number but got %s\n", pch);

	}
	else if (pch && !strcmp(pch, "rmdir"))
	{
		pch = strtok(0, " ");
		if (pch)
			fs_dirent_rmdir(current_directory, pch);
		else
			printf("rmdir: missing argument\n");
	}
	else if (pch && !strcmp(pch, "chdir"))
	{
		pch = strtok(0, " ");
		if (pch)
			current_directory = fs_dirent_namei(current_directory, pch);
		else
			printf("chdir: missing argument\n");
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
			"%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n",
			"Commands:",
			"echo <text>",
			"run <path>",
			"start <path>",
            "kill <pid>",
            "reap <pid>",
            "wait",
			"test <function>",
			"list",
			"time",
			"help",
			"exit",
            "stress",
			"mount <unit_no> <fs_type>",
			"format <unit_no> <fs_type>",
			"mkdir <dir>",
			"chdir <dir>",
			"rmdir <dir>"
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
	char line[HISTORY][1024];
    char cmd[1024] = {0};
    int y = 0;
    int x = 0;
	printf ("$ ");
	while(1)
	{
		char c = keyboard_read();
		if (x == 0 && c == ASCII_BS)
			continue;
		if (c == ASCII_CR)
		{
            strcpy(cmd, line[y]);
			printf("\n");
			int res = process_command(cmd);
			if (res < 0)
				break;
			x = 0;
			printf("$ ");
            int i;
            for (i = HISTORY-1; i > 0; i--) {
                strcpy(line[i], line[i-1]);
            }
		}
		else if (c == ASCII_BS)
		{
            console_putchar(c);
			x--;
		}
		else if (c >= 0x20 && c <= 0x7E) 
        {
            console_putchar(c);
			line[y][x]= c;
			x++;
		}
        else
        {
            if (c == KEY_DOWN) //down
            {
                if (y > 0) {
                    y--;
                    while (x > 0) {
                        console_putchar('\b');
                        x--;
                    }
                    printf ("\b\b$ ");
                    console_putstring(line[y]);
                    x = strlen(line[y]);
                }
            }
            else if (c == KEY_UP) //up
            {
                if (y < HISTORY-1) {
                    y++;
                    while (x > 0) {
                        console_putchar('\b');
                        x--;
                    }
                    printf ("\b\b$ ");
                    console_putstring(line[y]);
                    x = strlen(line[y]);
                }
            }
        }
		line[y][x] = 0;
	}
	return 0;
}
