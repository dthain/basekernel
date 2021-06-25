/*
Copyright (C) 2016-2019 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/

#include "kernel/types.h"
#include "library/stdio.h"
#include "library/syscalls.h"
#include "library/string.h"
#include "stdarg.h"

static char stdio_buffer[PAGE_SIZE] = { 0 };

static uint32_t stdio_buffer_index = 0;

void flush()
{
	syscall_object_write(KNO_STDOUT, stdio_buffer, stdio_buffer_index, 0);
	stdio_buffer_index = 0;
	stdio_buffer[0] = 0;
}

static void printf_buffer(char *s, unsigned len)
{
	while(len) {
		unsigned l = len % (PAGE_SIZE - 1);
		if(l > PAGE_SIZE - stdio_buffer_index - 1) {
			flush();
		}
		memcpy(stdio_buffer + stdio_buffer_index, s, l);
		stdio_buffer_index += l;
		len -= l;
	}
	stdio_buffer[stdio_buffer_index] = 0;
}

void printf_putchar(char c)
{
	printf_buffer(&c, 1);
	if(c == '\n')
		flush();
}

void printf_putstring(char *s)
{
	printf_buffer(s, strlen(s));
}
