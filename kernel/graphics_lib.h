/*
Copyright (C) 2017 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/

#ifndef GRAPHICS_LIB_H
#define GRAPHICS_LIB_H

typedef enum {GRAPHICS_END=0, GRAPHICS_WINDOW, GRAPHICS_COLOR, GRAPHICS_LINE, GRAPHICS_RECT, GRAPHICS_CLEAR, GRAPHICS_TEXT} graphics_command_t;

struct graphics_command {
    graphics_command_t type;
    int args[4];
};

#endif

