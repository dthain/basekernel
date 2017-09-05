#ifndef TERMINAL_H
#define TERMINAL_H

#include "kerneltypes.h"

int terminal_init(void);
int terminal_write(char *buffer, uint32_t size);
int terminal_read(char *buffer, uint32_t size);
int terminal_has_input();

#endif
