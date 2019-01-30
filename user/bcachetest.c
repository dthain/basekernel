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
  syscall_bcache_stats(&stats);
  printf("\nRead Hits: %d\nRead Misses: %d\nWrite Hits: %d\nWrite Misses: %d\n\n",
          stats.read_hits, stats.read_misses, stats.write_hits,
          stats.write_misses, stats.writebacks);

  return 0;
}
