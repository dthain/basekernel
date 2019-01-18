/*
Copyright (C) 2016-2019 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/

#include "kernel/gfxstream.h"
#include "kernel/types.h"
#include "library/user-io.h"
#include "library/syscalls.h"
#include "library/string.h"
#include "stdarg.h"

static char stdio_buffer[PAGE_SIZE] = { 0 };

static uint32_t stdio_buffer_index = 0;

static struct graphics_command graphics_buffer[PAGE_SIZE] = { {0} };

static uint32_t graphics_buffer_index = 0;

static int window_fd = KNO_STDWIN;

void flush()
{
	syscall_object_write(KNO_STDOUT, stdio_buffer, stdio_buffer_index);
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

static void draw_set_buffer(int t, int a0, int a1, int a2, int a3)
{
	struct graphics_command c = { t, {a0, a1, a2, a3} };
	graphics_buffer[graphics_buffer_index++] = c;
}

void draw_flush()
{
	draw_set_buffer(GRAPHICS_END, 0, 0, 0, 0);
	syscall_object_write(window_fd, graphics_buffer, graphics_buffer_index);
	graphics_buffer_index = 0;
}

void draw_window(int wd)
{
	window_fd = wd;
}

void draw_color(int r, int g, int b)
{
	draw_set_buffer(GRAPHICS_COLOR, r, g, b, 0);
}

void draw_rect(int x, int y, int w, int h)
{
	draw_set_buffer(GRAPHICS_RECT, x, y, w, h);
}

void draw_clear(int x, int y, int w, int h)
{
	draw_set_buffer(GRAPHICS_CLEAR, x, y, w, h);
}

void draw_line(int x, int y, int w, int h)
{
	draw_set_buffer(GRAPHICS_LINE, x, y, w, h);
}

void draw_string(int x, int y, char *s)
{
	draw_set_buffer(GRAPHICS_TEXT, x, y, (int) s, 0);
}
