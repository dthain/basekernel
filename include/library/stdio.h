/*
Copyright (C) 2016-2019 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/

#ifndef STDIO_H
#define STDIO_H

#include "kernel/types.h"

void printf_putchar(char c);
void printf_putstring(char *s);
void flush();

void draw_window(int wd);
void draw_fgcolor(int r, int g, int b);
void draw_bgcolor(int r, int g, int b);
void draw_rect(int x, int y, int w, int h);
void draw_clear(int x, int y, int w, int h);
void draw_line(int x, int y, int w, int h);
void draw_char(int x, int y, char c);
void draw_string(int x, int y, const char *s);
void draw_flush();

char window_getchar( int blocking );

#endif
