#include "shell.h"
#include "keyboard.h"
#include "console.h"
#include "string.h"
#include "rtc.h"
#include "syscall.h"
#include "kmalloc_test.h"

static int process_command(char *line)
{
	const char *pch = strtok(line, " ");
	if (pch && !strcmp(pch, "echo"))
	{
		pch = strtok(0, " ");
		if (pch) printf("%s\n", pch);
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
	else if (pch && !strcmp(pch, "exit"))
	{
		return -1;
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
	else if (pch)
	{
		printf("%s: command not found\n", pch);
	}
	return 0;
}

int shell_launch()
{
	char line[1024];
	char *pos = line;
	printf ("$ ");
	while(1)
	{
		char c = keyboard_read();
		if (pos == line && c == 8)
			continue;
		console_putchar(c);
		if (c == 13)
		{
			int res = process_command(line);
			if (res < 0)
				break;
			pos = line;
			printf("$ ");
		}
		else if (c == 8)
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
