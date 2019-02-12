/*
Copyright (C) 2016-2019 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/

#include "library/syscalls.h"
#include "library/string.h"

/*
  Tests the functionality of the syscall_bcache_stats(&bstats)
  system call and prints them out
*/

int main(int argc, char const *argv[]) {
  printf("Get the stats of the bcache after different events\n");

  struct bcache_stats stats;

  /* Initial Stats */
  syscall_bcache_flush();
  syscall_bcache_stats(&stats);
  printf("After Write\nRead Hits: %d\nRead Misses: %d\nWrite Hits: %d\nWrite Misses: %d\nWrite Backs: %d\n\n",
          stats.read_hits, stats.read_misses, stats.write_hits,
          stats.write_misses, stats.writebacks);

  /* Open file and write to it */
  syscall_chdir("/");
  int dir_fd = syscall_open_file("/", 0, 0);
  syscall_object_set_tag(dir_fd, "ROOT");
  int fd = syscall_open_file("ROOT:/data/jack", 2, 0);
  char buffer[100] = "Hello, world! I can write!!!!!!\n";
  syscall_object_write(fd, buffer, strlen(buffer));
  syscall_object_close(fd);

  syscall_bcache_flush();
  syscall_bcache_stats(&stats);
  printf("After Write\nRead Hits: %d\nRead Misses: %d\nWrite Hits: %d\nWrite Misses: %d\nWrite Backs: %d\n\n",
          stats.read_hits, stats.read_misses, stats.write_hits,
          stats.write_misses, stats.writebacks);

  /* Open file and read it */
  fd = syscall_open_file("ROOT:/data/jack", 1, 0);
  syscall_object_read(fd, buffer, strlen(buffer));
  printf("File says: %s\n", buffer);
  syscall_object_close(fd);

  syscall_bcache_flush();
  syscall_bcache_stats(&stats);
  printf("After Write\nRead Hits: %d\nRead Misses: %d\nWrite Hits: %d\nWrite Misses: %d\nWrite Backs: %d\n\n",
          stats.read_hits, stats.read_misses, stats.write_hits,
          stats.write_misses, stats.writebacks);

  syscall_object_close(dir_fd);

  return 0;
}
