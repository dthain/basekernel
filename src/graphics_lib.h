/*
Copyright (C) 2017 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/

#ifndef GRAPHICS_LIB_H
#define GRAPHICS_LIB_H

enum command_type {END=0, WINDOW, COLOR, LINE, RECT, CLEAR, TEXT};

struct gfx_command {
    enum command_type type;
    int args[4];
};

#endif

