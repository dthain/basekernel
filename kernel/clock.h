/*
Copyright (C) 2015-2019 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/

#ifndef CLOCK_H
#define CLOCK_H

#include "kernel/types.h"

typedef struct {
	uint32_t seconds;
	uint32_t millis;
} clock_t;

void clock_init();
clock_t clock_read();
clock_t clock_diff(clock_t start, clock_t stop);
void clock_wait(uint32_t millis);

#endif
