/*
Copyright (C) 2016 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/

#ifndef MOUSE_H
#define MOUSE_H

#include "kerneltypes.h"

struct mouse_event {
	uint8_t buttons;
	int32_t x;
	int32_t y;
};

void mouse_read( struct mouse_event *e );
void mouse_init();

#endif
