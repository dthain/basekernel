#ifndef ERRNO_H
#define ERRNO_H

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

const char *strerror(int err_code);

#endif
