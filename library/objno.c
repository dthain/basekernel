#include "kernel/ktypes.h"
#include "library/objno.h"
#include "library/string.h"

static const char KOBJECT_INVALID_STRING[] = "Invalid";
static const char KOBJECT_FILE_STRING[] = "File";
static const char KOBJECT_DEVICE_STRING[] = "Device";
static const char KOBJECT_GRAPHICS_STRING[] = "Graphics";

const char *strobjno(int err_code)
{
	switch(err_code) {
		case KOBJECT_INVALID:
			return KOBJECT_INVALID_STRING;
		case KOBJECT_FILE:
			return KOBJECT_FILE_STRING;
		case KOBJECT_DEVICE:
			return KOBJECT_DEVICE_STRING;
		case KOBJECT_GRAPHICS:
			return KOBJECT_GRAPHICS_STRING;
		default:
			return "unknown";
	}
}
