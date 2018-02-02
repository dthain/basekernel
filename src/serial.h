#ifndef SERIAL_H
#define SERIAL_H
void serial_init();
char serial_read();
void serial_write(char a);
void serial_write_string(char *s);
#endif
