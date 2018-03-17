#include "syscalls.h"
#include "string.h"

int main( const char *argv[], int argc )
{
  int x = fork();
  printf("hello world, I am %d.\n", process_self());
  printf("My fork returned %d\n", x);

  printf("Some numbers:\n");

  int i;
  for (i = 0; i < 5; ++i) {
      printf("%d\n", i);
  }
	return 0;
}
