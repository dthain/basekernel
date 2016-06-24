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

void console_init( struct graphics *g );
void console_putchar( char c );
int  console_write( int unit, const void *buffer, int nblocks, int offset );
void console_heartbeat();

#define console_printf printf

#endif
