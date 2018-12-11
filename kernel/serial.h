#ifndef SERIAL_H
#define SERIAL_H

void serial_init();

int serial_read( int unit, char *data, int length );
int serial_write( int unit, const char *data, int length );

#endif
