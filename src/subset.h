#ifndef SUBSET_H
#define SUBSET_H

#include "fs.h"

#define MAX_FS_SPACES 1048

#define NS_READ 1
#define NS_WRITE 2
#define NS_EXECUTE 4

struct fs_space {
  bool present;
	struct fs_dirent *d;
	uint32_t count;
  //May later need to implement a "valid" to tell if deleted
};

struct fs_space_ref {
  char * name;
  int perms;
  uint32_t gindex;
};

extern struct fs_space * spaces;
extern int used_fs_spaces;

#endif
