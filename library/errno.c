/*
Copyright (C) 2016-2019 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/

#include "kernel/error.h"
#include "library/errno.h"
#include "library/string.h"

const char * strerror( kernel_error_t err_code) {
	switch (err_code) {
		case KERROR_NOT_FOUND:
			return "Not Found";
		case KERROR_INVALID_REQUEST:
			return "Invalid Request";
		case KERROR_PERMISSION_DENIED:
			return "Permission Denied";
		case KERROR_NOT_IMPLEMENTED:
			return "Not Implemented";
		case KERROR_NOT_EXECUTABLE:
			return "Not Executable";
		case KERROR_EXECUTION_FAILED:
			return "Execution Failed";
		case KERROR_NOT_A_DIRECTORY:
			return "Not a Directory";
		case KERROR_NOT_A_FILE:
			return "Not a File";
		case KERROR_NOT_A_WINDOW:
			return "Not a Window";
		case KERROR_NOT_A_DEVICE:
			return "Not a Device";
		case KERROR_NOT_A_CONSOLE:
			return "Not a Console";
		case KERROR_NOT_A_PIPE:
			return "Not a Pipe";
		case KERROR_INVALID_SYSCALL:
			return "Invalid System Call";
		case KERROR_INVALID_OBJECT:
			return "Invalid Object";
		case KERROR_INVALID_ADDRESS:
			return "Invalid Address";
		case KERROR_INVALID_PATH:
			return "Invalid Path";
		case KERROR_INVALID_TAG:
			return "Invalid Tag";
		case KERROR_OUT_OF_MEMORY:
			return "Out of Memory";
		case KERROR_OUT_OF_OBJECTS:
			return "Out of Objects";
		case KERROR_OUT_OF_SPACE:
			return "Out of Space";
		default:
			return "Unknown error";
	}
}
