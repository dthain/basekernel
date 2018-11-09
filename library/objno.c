#include "kernel/ktypes.h"
#include "library/objno.h"
#include "library/string.h"

const char *strerror(int err_code)
{
	if(err_code == KOBJECT_INVALID) {
		return KOBJECT_INVALID_STRING;
	}
	if(err_code == KOBJECT_FILE) {
		return KOBJECT_FILE_STRING;
	}
	if(err_code == KOBJECT_DEVICE) {
		return KOBJECT_DEVICE_STRING;
	}
	if(err_code == KOBJECT_GRAPHICS) {
		return KOBJECT_GRAPHICS_STRING;
	}
}
