#include "fs_space.h"
#include "string.h"
#include "kmalloc.h"

int fs_space_depth_check(const char *path, int cdepth) {
  //Returns new depth, or -1 on depth going negative.
  if (path[0] == '/') {
    cdepth = 0;
    path += 1;
  }
  char *lpath = kmalloc(strlen(path)+1);
  strcpy(lpath, path);
  char *part = strtok(lpath,"/");
  while(part && cdepth >= 0) {
    if (!part) {
      break;
    } else if (!strcmp(part, "..")) {
      cdepth--;
    } else {
      cdepth++;
    }
  part = strtok(0,"/");
  }
  kfree(lpath);
  if (cdepth < 0) {
    return -1;
  }
  return cdepth;
}

struct fs_space * fs_spaces = 0;
int fs_spaces_used = 0;
