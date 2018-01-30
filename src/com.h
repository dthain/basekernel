#ifndef COM_H
#define COM_H
void init_serial();
char read_serial();
void write_serial(char a);
void write_string_serial(char *s);
#endif
