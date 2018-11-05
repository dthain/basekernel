#include "library/syscalls.h"
#include "library/string.h"
#include "library/user-io.h"


int main(const char **argv, int argc)
{
	char * type_string = "A simple type string";

	printf("Generating/typing all file descriptors.\n");
	int type = -2;
	int intent = -3;

	int window_descriptor = open_window(KNO_STDWIN, 1, 1, 1, 1);
	if(!window_descriptor) {
		return 1;
	}

	printf("This object is: %s\n", type_string);
	
	object_set_intent(window_descriptor, type_string);

	int last_descriptor = process_highest_fd();
	// First a test of highest_fd()
	printf("Highest allocated FD: %d\nLast Descriptor: %d\n",
			window_descriptor, last_descriptor);

	for (int descriptor = 0; descriptor <= last_descriptor; descriptor++) 
	{
		type = object_type(descriptor);
		intent = object_get_intent(descriptor);
		if (intent != 0) {
			printf("FD: %d is of type: %d, with intent: \"%s\"\n", descriptor, type, intent);
		}
		if (intent == 0) {
			printf("FD: %d is of type: %d, with intent %d\n", descriptor, type, intent);
		}
	}
	printf("This object is: %s\n", type_string);

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
