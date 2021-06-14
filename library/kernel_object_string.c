/*
Copyright (C) 2016-2019 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/

#include "kernel/types.h"
#include "library/kernel_object_string.h"
#include "library/string.h"

const char *kernel_object_string( kobject_type_t type )
{
	switch(type) {
		case KOBJECT_FILE:
			return "file";
		case KOBJECT_DIR:
			return "directory";	
		case KOBJECT_DEVICE:
			return "device";
		case KOBJECT_WINDOW:
			return "window";
		case KOBJECT_CONSOLE:
			return "console";
		case KOBJECT_PIPE:
			return "pipe";
		default:
			return "unknown";
	}
}
