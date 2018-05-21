/*
Copyright (C) 2015 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/

#ifndef CONSOLE_H
#define CONSOLE_H

#include "kerneltypes.h"
#include "string.h"
#include "graphics.h"
#include "device.h"

struct device * console_get();
struct device * console_init( struct graphics *g );
struct device * console_create( struct graphics *g );
void console_putchar( char c );
void console_putstring( const char *c );
int  console_write( int unit, const void *buffer, int nblocks, int offset );
void console_heartbeat();
void printf_putchar( char c );
void printf_putstring( char *s );

#define console_printf printf

#endif
