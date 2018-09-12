#include "library/syscalls.h"
#include "library/user-io.h"

int main(const char ** argv, int argc) {
  int window_descriptor = open_window(KNO_STDWIN, 1, 1, 1, 1);
  if (window_descriptor) {
    return 1;
  }

  printf("Window file is of type: %d\n", file_describe(window_descriptor));

  return 0;
}
