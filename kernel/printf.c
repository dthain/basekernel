/*
Copyright (C) 2016-2019 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/

#include "printf.h"
#include "string.h"
#include "console.h"
#include "keyboard.h"
#include <stdarg.h>

static void printf_putchar( char c )
{
	console_putchar(&console_root,c);
}

char getchar()
{
	return keyboard_read(0);
}

void putchar( char c)
{
	return printf_putchar(c);
}

static void printf_putstring(char *s)
{
	console_putstring(&console_root,s);
}

static void printf_puthexdigit(uint8_t i)
{
	if(i < 10) {
		printf_putchar('0' + i);
	} else {
		printf_putchar('a' + i - 10);
	}
}

static void printf_puthex(uint32_t i)
{
	int j;
	for(j = 28; j >= 0; j = j - 4) {
		printf_puthexdigit((i >> j) & 0x0f);
	}
}

static void printf_putint(int32_t i)
{
	int f, d;
	if(i < 0 && i != 0) {
		printf_putchar('-');
		i = -i;
	}

	f = 1;
	while((i / f) >= 10) {
		f *= 10;
	}
	while(f > 0) {
		d = i / f;
		printf_putchar('0' + d);
		i = i - d * f;
		f = f / 10;
	}
}

static void printf_putuint(uint32_t u)
{
	int f, d;
	f = 1;
	while((u / f) >= 10) {
		f *= 10;
	}
	while(f > 0) {
		d = u / f;
		printf_putchar('0' + d);
		u = u - d * f;
		f = f / 10;
	}
}

void printf(const char *s, ...)
{
	va_list args;

	uint32_t u;
	int32_t i;
	char *str;

	va_start(args, s);

	while(*s) {
		if(*s != '%') {
			printf_putchar(*s);
		} else {
			s++;
			switch (*s) {
			case 'd':
				i = va_arg(args, int32_t);
				printf_putint(i);
				break;
			case 'u':
				u = va_arg(args, uint32_t);
				printf_putuint(u);
				break;
			case 'x':
				u = va_arg(args, uint32_t);
				printf_puthex(u);
				break;
			case 's':
				str = va_arg(args, char *);
				printf_putstring(str);
				break;
			case 'c':
				u = va_arg(args, int32_t);
				printf_putchar(u);
				break;
			case 0:
				return;
				break;
			default:
				printf_putchar(*s);
				break;
			}
		}
		s++;
	}
	va_end(args);
}

