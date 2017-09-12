#include "terminal.h"
#include "kerneltypes.h"
#include "file.h"
#include "syscall.h"
#include "process.h"

#define TERMINAL_FILE "tty"
#define TERMINAL_BUFFER_SIZE 1024

int terminal_written = 0;

struct list queue = {0, 0};

int terminal_init()
{
	register_memory_inode(TERMINAL_FILE);
	return 0;
}

int terminal_write(char *buffer, uint32_t length)
{
	uint32_t i = 0;
	int fd = open(TERMINAL_FILE, FILE_MODE_WRITE, 0);
	while (i < length)
	{
		while (terminal_written) process_wait(&queue);
		uint32_t n = length - i < TERMINAL_BUFFER_SIZE ? length - i : TERMINAL_BUFFER_SIZE;
		write(fd, buffer + i, n);
	}
	close(fd);
	return length;
}

int terminal_read(char *buffer, uint32_t length)
{
	uint32_t i = 0;
	if (length > TERMINAL_BUFFER_SIZE)
		length = TERMINAL_BUFFER_SIZE;
	int fd = open(TERMINAL_FILE, FILE_MODE_WRITE, 0);
	write(fd, buffer + i, length);
	close(fd);
	terminal_written = 0;
	return length;
}

int terminal_has_input()
{
	return terminal_written;
}
