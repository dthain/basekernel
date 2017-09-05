/*
Copyright (C) 2015 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/

#ifndef KEYBOARD_H
#define KEYBOARD_H

#include "kerneltypes.h"

int keyboard_read(char *out_buffer, uint32_t len);
void keyboard_init();

#endif
