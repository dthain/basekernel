/*
Copyright (C) 2016-2019 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/

#include "kernel/types.h"
#include "library/objno.h"
#include "library/string.h"

const char *strobjno( kobject_type_t type )
{
	switch(type) {
		case KOBJECT_FILE:
			return "file";
		case KOBJECT_DIR:
			return "directory";	
		case KOBJECT_DEVICE:
			return "device";
		case KOBJECT_GRAPHICS:
			return "graphics";
		case KOBJECT_CONSOLE:
			return "console";
		case KOBJECT_PIPE:
			return "pipe";
		default:
			return "unknown";
	}
}
