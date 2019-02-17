#include "library/errno.h"
#include "library/syscalls.h"
#include "library/string.h"
#include "library/user-io.h"
#include "library/malloc.h"

#define TAG_BUFFER_SIZE 256

int main(int argc, char *argv[])
{
	char *tag_value = "A simple tag string";

	printf("Generating/typing all file descriptors.\n");
	int type = -2;
	int tag = -3;

	int window_descriptor = syscall_open_window(KNO_STDWIN, 1, 1, 1, 1);
	if(!window_descriptor) {
		return 1;
	}

	syscall_object_set_tag(window_descriptor, tag_value);

	int last_descriptor = syscall_object_max();
	printf("Highest allocated FD: %d\nLast Descriptor: %d\n", window_descriptor, last_descriptor);

	char *tag_string = malloc(sizeof(char) * TAG_BUFFER_SIZE);

	for(int descriptor = 0; descriptor <= last_descriptor; descriptor++) {
		type = syscall_object_type(descriptor);
		tag = syscall_object_get_tag(descriptor, tag_string, TAG_BUFFER_SIZE);
		printf("FD: %d is of type: %d, with tag: ", descriptor, type);
		if(tag != 0) {
			printf("\"%s\"\n", tag_string);
		}
		if(tag == 0) {
			printf("%d\n", tag);
		}
	}

	return 0;
}
