#include "library/malloc.h"
#include "library/syscalls.h"
#include "library/string.h"
#include "library/user-io.h"

/*
  Continuously read files at random intervals to generate statistics
*/

int filereads() {
  int fd[8];

  char buffer[4096];
  int i, j;
  int n, count = 0, mod = 7;

  while (1) {
    for (i = 0; i < (count + mod) % mod; i++) {
      fd[0] = syscall_open_file("bin/exectest.exe", 0, 0);
      fd[1] = syscall_open_file("bin/long.exe", 0, 0);
      fd[2] = syscall_open_file("bin/saver.exe", 0, 0);
      for (j = 0; j < 3; j++) {
        while((n = syscall_object_read(fd[j], buffer, 1024)) > 0) {
        }
      }

      syscall_object_close(fd[0]);
      syscall_object_close(fd[1]);
      syscall_object_close(fd[2]);
    }
    syscall_process_sleep(3000);
    count++;
  }
}

int main(int argc, char const *argv[]) {

  if (argc < 2 || !strcmp(argv[1], "bcache")) {
    filereads();
  }
  else if (!strcmp(argv[1], "process")) {
    filereads();
  }
  else if (!strcmp(argv[1], "device_driver")) {
    filereads();
  }
  else if (!strcmp(argv[1], "system")) {
    filereads();
  }
  else {
    filereads();
  }

  return 0;
}
