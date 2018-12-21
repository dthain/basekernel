#include "library/errno.h"
#include "library/syscalls.h"
#include "library/string.h"
#include "library/user-io.h"
#include "library/malloc.h"

#define INTENT_BUFFER_SIZE 256

int main(const char **argv, int argc)
{
	char *intent_value = "A simple intent string";

	printf("Generating/typing all file descriptors.\n");
	int type = -2;
	int intent = -3;

	int window_descriptor = syscall_open_window(KNO_STDWIN, 1, 1, 1, 1);
	if(!window_descriptor) {
		return 1;
	}

	syscall_object_set_intent(window_descriptor, intent_value);

	int last_descriptor = syscall_object_max();
	printf("Highest allocated FD: %d\nLast Descriptor: %d\n", window_descriptor, last_descriptor);

	char *intent_string = malloc(sizeof(char) * INTENT_BUFFER_SIZE);

	for(int descriptor = 0; descriptor <= last_descriptor; descriptor++) {
		type = syscall_object_type(descriptor);
		intent = syscall_object_get_intent(descriptor, intent_string, INTENT_BUFFER_SIZE);
		printf("FD: %d is of type: %d, with intent: ", descriptor, type);
		if(intent != 0) {
			printf("\"%s\"\n", intent_string);
		}
		if(intent == 0) {
			printf("%d\n", intent);
		}
	}

	return 0;
}
