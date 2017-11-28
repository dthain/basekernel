/*
Copyright (C) 2017 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/

#include "user-io.h"
#include "kerneltypes.h"
#include "syscalls.h"
#include "string.h"
#include "stdarg.h"

static char stdio_buffer[PAGE_SIZE] = {0};
static uint32_t stdio_buffer_index = 0;

static char graphics_buffer[PAGE_SIZE] = {0};
static uint32_t graphics_buffer_index = 0;

static void flush()
{
    debug(stdio_buffer);
    stdio_buffer_index = 0;
    stdio_buffer[0] = 0;
}

static void printf_buffer( char *s, unsigned len )
{
    while (len)
    {
        unsigned l = len % (PAGE_SIZE - 1);
        if (l > PAGE_SIZE - stdio_buffer_index - 1)
        {
            flush();
        }
        memcpy(stdio_buffer + stdio_buffer_index, s, l);
        stdio_buffer_index += l;
        len -= l; 
    }
    stdio_buffer[stdio_buffer_index] = 0;
}

void printf_putchar( char c )
{
    printf_buffer(&c, 1);
    if (c == '\n') flush();
}

void printf_putstring( char *s )
{
    printf_buffer(s, strlen(s));
}

static void flush_graphics()
{
    draw_write(graphics_buffer);
    graphics_buffer_index = 0;
    graphics_buffer[0] = 0;
}

static void printf_buffer_graphics( char *s, unsigned len )
{
    while (len)
    {
        unsigned l = len % (PAGE_SIZE - 1);
        if (l > PAGE_SIZE - graphics_buffer_index - 1)
        {
            flush_graphics();
        }
        memcpy(graphics_buffer + graphics_buffer_index, s, l);
        graphics_buffer_index += l;
        len -= l; 
    }
    graphics_buffer[graphics_buffer_index] = 0;
}

static void printf_putchar_graphics( char c )
{
    printf_buffer_graphics(&c, 1);
}

static void printf_putstring_graphics( char *s )
{
    printf_buffer_graphics(s, strlen(s));
}

static void printf_putint_graphics( int32_t i )
{
	int f, d;
	if(i<0 && i!=0) {
		printf_putchar_graphics('-');
		i=-i;
	}

	f = 1;
	while((i/f) >= 10) {
		f*=10;
	}
	while(f>0) {
		d = i/f;
		printf_putchar_graphics('0'+d);
		i = i-d*f;
		f = f/10;
	}
}

void printf_graphics( const char *s, ... )
{
	va_list args;

	int32_t i;
	char *str;

	va_start(args,s);

	while(*s) {
		if(*s!='%') {
			printf_putchar_graphics(*s);
		} else {
			s++;
			switch(*s) {
				case 'd':
					i = va_arg(args,int32_t);
					printf_putint_graphics(i);
					break;
				case 's':
					str = va_arg(args,char*);
					printf_putstring_graphics(str);
					break;
				case 0:
					return;
					break;
				default:
					printf_putchar_graphics(*s);
					break;
			}
		}
		s++;
	}
    flush_graphics();
	va_end(args);
}
