/*
Copyright (C) 2016-2019 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/

#ifndef USERIO_H
#define USERIO_H

#define KNO_STDIN  0
#define KNO_STDOUT 1
#define KNO_STDERR 2
#define KNO_STDWIN 3

void printf_putchar(char c);
void printf_putstring(char *s);
void flush();

void draw_window(int wd);
void draw_color(int r, int g, int b);
void draw_rect(int x, int y, int w, int h);
void draw_clear(int x, int y, int w, int h);
void draw_line(int x, int y, int w, int h);
void draw_char(int x, int y, char c);
void draw_string(int x, int y, char *s);
void draw_flush();

#endif
