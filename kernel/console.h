/*
Copyright (C) 2015-2019 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/

#ifndef CONSOLE_H
#define CONSOLE_H

#include "kernel/types.h"
#include "graphics.h"
#include "device.h"
#include "string.h"

/*
console_init creates the very first global console that
is used for kernel debug output.  The singleton "console_root"
must be statically allocated so that is usable at early
startup before memory allocation is available.
*/

extern struct console console_root;
struct console * console_init( struct graphics *g );

/*
Any number of other consoles can be created and manipulated
on top of existing windows.
*/

struct console * console_create( struct graphics *g );
void console_delete( struct console *c );
int  console_write( struct console *c, const char *data, int length );
void console_putchar( struct console *c, char ch );
void console_putstring( struct console *c, const char *str );
void console_heartbeat( struct console *c );
void console_size( struct console *c, int *xsize, int *ysize );
struct console *console_addref( struct console *c );

#endif
