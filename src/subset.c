#include "syscalls.h"
#include "string.h"
#include "user-io.h"
int main(char * argv[], int argc) {
  if (argc == 2) {
    int wd = draw_create(0, 600, 500, 200, 100);
    if (wd < 0) {
      printf("Window failure\n");
      exit(1);
    }
    const char* argv[] = {"SUBSET.EXE"};
    int pid = process_run_subset("SUBSET.EXE", argv, 1, wd);
    printf("subset pid: %d\n", pid);
  } else {
    int x = 0;
    int y = 0;
    int maxx = 200;
    int maxy = 100;
    draw_clear(0, 0, maxx, maxy);
    while (1) {
      draw_window(0);
      draw_color(0, 200, 0);
      draw_rect(0, 0, x, y);
      draw_flush();
      x += 5;
      y += 5;
      x %= maxx;
      y %= maxy;
      if (x < 10) {
        draw_window(0);
        draw_clear(0, 0, maxx, maxy);
        draw_flush();
      }
      sleep(100);
    }
  }
  return 0;
}
