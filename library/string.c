/*
Copyright (C) 2015-2019 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/

#include "kernel/types.h"
#include "kernel/ascii.h"
#include "library/malloc.h"
#include "library/string.h"
#include "library/malloc.h"
#include "stdarg.h"

void strcpy(char *d, const char *s)
{
	while(*s) {
		*d++ = *s++;
	}
	*d = 0;
}

void strncpy(char *d, const char *s, unsigned length)
{
	while(*s && length--) {
		*d++ = *s++;
	}
	*d = 0;
}

char * strdup(const char *s)
{
	char * d = (char *)malloc((strlen(s)+1) * sizeof(char));
	char * tmp = d;
	while(*s) {
		*tmp++ = *s++;
	}
	*tmp = 0;
	return d;
}

int strcmp(const char *a, const char *b)
{
	while(1) {
		if(*a < *b) {
			return -1;
		} else if(*a > *b) {
			return 1;
		} else if(*a == 0) {
			return 0;
		} else {
			a++;
			b++;
		}
	}
}

int strncmp(const char *a, const char *b, unsigned length)
{
	while(length > 0) {
		if(*a < *b) {
			return -1;
		} else if(*a > *b) {
			return 1;
		} else if(*a == 0) {
			return 0;
		} else {
			a++;
			b++;
			length--;
		}
	}
	return 0;
}

unsigned strlen(const char *s)
{
	unsigned len = 0;
	while(*s) {
		len++;
		s++;
	}
	return len;
}

char *strrev(char *s)
{
	unsigned start = 0;
	unsigned end = strlen(s) - 1;
	char swap;

	while(start < end) {
		swap = s[start];
		s[start] = s[end];
		s[end] = swap;

		start++;
		end--;
	}

	return s;
}

char *strcat(char *d, const char *s)
{
	strcpy(d + strlen(d), s);
	return d;
}

const char *strchr(const char *s, char ch)
{
	while(*s) {
		if(*s == ch)
			return s;
		s++;
	}
	return 0;
}

char *strtok(char *s, const char *delim)
{
	static char *oldword = 0;
	char *word;

	if(!s)
		s = oldword;

	while(*s && strchr(delim, *s))
		s++;

	if(!*s) {
		oldword = s;
		return 0;
	}

	word = s;
	while(*s && !strchr(delim, *s))
		s++;

	if(*s) {
		*s = 0;
		oldword = s + 1;
	} else {
		oldword = s;
	}

	return word;
}

int str2int(const char *s, int *d)
{
	int val = 0;
	for(; *s; ++s) {
		val *= 10;
		if(*s > ASCII_9 || *s < ASCII_0) {
			return 0;
		}
		val += (*s - '0');
	}
	*d = val;
	return 1;
}

void memset(void *vd, char value, unsigned length)
{
	char *d = vd;
	while(length) {
		*d = value;
		length--;
		d++;
	}
}

void memcpy(void *vd, const void *vs, unsigned length)
{
	char *d = vd;
	const char *s = vs;
	while(length) {
		*d = *s;
		d++;
		s++;
		length--;
	}
}

extern void printf_putstring(const char *str);
extern void printf_putchar(char c);

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

char *uint_to_string(uint32_t u, char *s)
{
	uint32_t f, d, i;

	f = 1;
	i = 0;
	while((u / (f * 10)) > 0) {
		f *= 10;
	}
	while(f > 0) {
		d = u / f;
		s[i] = '0' + d;
		u = u % f;
		f = f / 10;
		i++;
	}
	s[i] = 0;
	return s;
}
