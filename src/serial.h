#ifndef SERIAL_H
#define SERIAL_H
#include "kerneltypes.h"

void serial_init();
char serial_read(uint8_t port_no);
void serial_write(uint8_t port_no, char a);
void serial_write_string(uint8_t port_no, char *s);
#endif
