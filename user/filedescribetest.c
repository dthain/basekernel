#include "library/errno.h"
#include "library/syscalls.h"
#include "library/string.h"
#include "library/user-io.h"

int main(const char **argv, int argc)
{
	int type = -1;
	int window_descriptor = open_window(KNO_STDWIN, 1, 1, 1, 1);
	if(!window_descriptor) {
		return 1;
	}
	type = object_type(window_descriptor);
	printf("Window file %d is of type: %d\n", window_descriptor, type);
	
	char * type_string = malloc(256);
	char * type_default = "a type string";

	printf("Displaying all type strings\n");
	//strerror(-1, type_string);
	printf("%s\n", type_default);
	strcpy(type_string, type_default);
	printf("%s\n", type_string);

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
