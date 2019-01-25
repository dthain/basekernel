/*
Copyright (C) 2015-2019 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/

#ifndef PIC_H
#define PIC_H

#include "kernel/types.h"

void pic_init(int base0, int base1);
void pic_enable(uint8_t irq);
void pic_disable(uint8_t irq);
void pic_acknowledge(uint8_t irq);

#endif
