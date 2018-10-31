#include "library/syscalls.h"
#include "library/string.h"
#include "library/user-io.h"

enum {
	KOBJECT_INVALID = 0,
	KOBJECT_FILE,
	KOBJECT_DEVICE,
	KOBJECT_GRPAHICS,
	KOBJECT_PIPE
};

int main(const char **argv, int argc)
{
	printf("Generating/typing all file descriptors.\n");
	int type = -2;
	int window_descriptor = open_window(KNO_STDWIN, 1, 1, 1, 1);
	if(!window_descriptor) {
		return 1;
	}
	type = object_type(window_descriptor);
	printf("Window file %d is of type: %d\n", window_descriptor, type);

	/*
	 * Presently, the following code returns the same error as the file open
	 * demo.
	 * Can't be used until that bug is addressed.
	 int reg_file_descriptor = open("testfile.txt", 1, 0);
	 type = object_type(reg_file_descriptor);
	 printf("Text file %d is of type: %d\n", reg_file_descriptor, type);
	 */

	return 0;
}
