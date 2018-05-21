#ifndef FS_SPACE_H
#define FS_SPACE_H

#include "fs.h"

#define MAX_FS_SPACES 512

#define NS_READ 1
#define NS_WRITE 2
#define NS_EXECUTE 4

struct fs_space {
  bool present;
	struct fs_dirent *d;
	uint32_t count;
};

struct fs_space_ref {
  char * name;
  int perms;
  uint32_t gindex;
};

int fs_space_depth_check(const char *path, int cdepth);

extern struct fs_space * fs_spaces;
extern int fs_spaces_used;

#endif
