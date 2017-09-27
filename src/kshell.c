#include "kshell.h"
#include "keyboard.h"
#include "console.h"
#include "string.h"
#include "rtc.h"
#include "syscall.h"
#include "cdromfs.h"
#include "kmalloc.h"
#include "process.h"
#include "kmalloc.h"
#include "ascii.h"

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

static int list_directory( const char *path )
{
	struct cdrom_volume *v = cdrom_volume_open(1);
	if(v) {
		struct cdrom_dirent *d = cdrom_volume_root(v);
		if(d) {
			int buffer_length = 1024;
			char *buffer = kmalloc(buffer_length);
			if(buffer) {
				int length = cdrom_dirent_read_dir(d,buffer,buffer_length);
				print_directory(buffer,length);
				kfree(buffer);
			}
			cdrom_dirent_close(d);
		} else {
			printf("couldn't access root dir!\n");
		}
		cdrom_volume_close(v);
	} else {
		printf("couldn't mount filesystem!\n");
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
	else if (pch && !strcmp(pch, "run"))
	{
		pch = strtok(0, " ");
		if (pch) {
			sys_run(pch);
			process_yield();
		}
		else
			list_directory("run: missing argument");
	}
	else if (pch && !strcmp(pch, "list"))
	{
		pch = strtok(0, " ");
		if (pch)
			printf("%s: unexpected argument\n", pch);
		else
			list_directory("/");

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
			"%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n",
			"Commands:",
			"echo <text>",
			"run <path>",
			"test <function>",
			"list",
			"time",
			"help",
			"exit"
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
