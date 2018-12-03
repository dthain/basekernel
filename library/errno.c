#include "kernel/error.h"
#include "library/errno.h"
#include "library/string.h"

static const char KERROR_NOT_FOUND_STRING[] = "KERROR: Not Found";
static const char KERROR_INVALID_REQUEST_STRING[] = "KERROR: Invalid Request";
static const char KERROR_PERMISSION_DENIED_STRING[] = "KERROR: Permission Denied";
static const char KERROR_NOT_IMPLEMENTED_STRING[] = "KERROR: Not Implemented";
static const char KERROR_NOT_EXECUTABLE_STRING[] = "KERROR: Not Executable";
static const char KERROR_EXECUTION_FAILED_STRING[] = "KERROR: Execution Failed";
static const char KERROR_NOT_SUPPORTED_STRING[] = "KERROR: Not a Supported String";
static const char KERROR_NOT_A_DIRECTORY_STRING[] = "KERROR: Not a Directory";
static const char KERROR_NOT_A_FILE_STRING[] = "KERROR: Not a File";
static const char KERROR_NOT_A_WINDOW_STRING[] = "KERROR: Not a Window";
static const char KERROR_NOT_A_DEVICE_STRING[] = "KERROR: Not a Device";
static const char KERROR_NO_MEMORY_STRING[] = "KERROR: No Memory";
static const char KERROR_IO_FAILURE_STRING[] = "KERROR: IO Failure";

const char * strerror(int err_code) {
	switch (err_code) {
		case KERROR_NOT_FOUND:
			return KERROR_NOT_FOUND_STRING;
		case KERROR_INVALID_REQUEST:
			return KERROR_INVALID_REQUEST_STRING;
		case KERROR_PERMISSION_DENIED:
			return KERROR_PERMISSION_DENIED_STRING;
		case KERROR_NOT_IMPLEMENTED:
			return KERROR_NOT_IMPLEMENTED_STRING;
		case KERROR_NOT_EXECUTABLE:
			return KERROR_NOT_EXECUTABLE_STRING;
		case KERROR_EXECUTION_FAILED:
			return KERROR_EXECUTION_FAILED_STRING;
		case KERROR_NOT_SUPPORTED:
			return KERROR_NOT_SUPPORTED_STRING;
		case KERROR_NOT_A_DIRECTORY:
			return KERROR_NOT_A_DIRECTORY_STRING;
		case KERROR_NOT_A_FILE:
			return KERROR_NOT_A_FILE_STRING;
		case KERROR_NOT_A_WINDOW:
			return KERROR_NOT_A_WINDOW_STRING;
		case KERROR_NOT_A_DEVICE:
			return KERROR_NOT_A_DEVICE_STRING;
		case KERROR_NO_MEMORY:
			return KERROR_NO_MEMORY_STRING;
		case KERROR_IO_FAILURE:
			return KERROR_IO_FAILURE_STRING;
	}
}
