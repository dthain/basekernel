#include "string.h"
#include "syscalls.h"
#include "user-io.h"
#include "ascii.h"

int process_command(char * line) {
  printf("processing command...\n");
  return 0;
}

int main(char ** argv, int argc) {
  printf("User shell ready:\n");
  char line[1024];
  char * pos = line;
  char c;
  printf("$ ");
  while (1) {
    flush();
    c = keyboard_read_char();
    if (pos == line && c == ASCII_BS)
      continue;
    printf_putchar(c);
    flush();
    if (c == ASCII_CR) {
      int res = process_command(line);
      if (res < 0)
        break;
      pos = line;
      printf("$ ");
    }
    else if (c == ASCII_BS) {
      pos--;
    }
    else {
      *pos = c;
      pos++;
    }
    *pos = 0;
  }
  return 0;
}
