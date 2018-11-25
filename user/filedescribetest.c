#include "library/errno.h"
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

	int last_descriptor = process_object_max();
	// First a test of highest_fd()
	printf("Highest allocated FD: %d\nLast Descriptor: %d\n",
			window_descriptor, last_descriptor);

	for (int descriptor = 0; descriptor <= last_descriptor; descriptor++) 
	{
		type = object_type(descriptor);
		intent = object_get_intent(descriptor);
		printf("FD: %d is of type: %d, with intent: ", descriptor, type);
		if (intent != 0) {
			printf("\"%s\"\n", intent);
		}
		if (intent == 0) {
			printf("NULL\n");
		}
	}

	return 0;
}
