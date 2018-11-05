#ifndef ERRNO_H
#define ERRNO_H

const char * KERROR_NOT_FOUND_STRING = "KERROR: Not Found";
const char * KERROR_INVALID_REQUEST_STRING = "KERROR: Invalid Request";
const char * KERROR_PERMISSION_DENIED_STRING = "KERROR: Permission Denied";
const char * KERROR_NOT_IMPLEMENTED_STRING = "KERROR: Not Implemented";
const char * KERROR_NOT_EXECUTABLE_STRING = "KERROR: Not Executable";
const char * KERROR_EXECUTION_FAILED_STRING = "KERROR: Execution Failed";
const char * KERROR_NOT_SUPPORTED_STRING = "KERROR: Not a Supported String";
const char * KERROR_NOT_A_DIRECTORY_STRING = "KERROR: Not a Directory";
const char * KERROR_NOT_A_FILE_STRING = "KERROR: Not a File";
const char * KERROR_NOT_A_WINDOW_STRING = "KERROR: Not a Window";
const char * KERROR_NOT_A_DEVICE_STRING = "KERROR: Not a Device";
const char * KERROR_NO_MEMORY_STRING = "KERROR: No Memory";
const char * KERROR_IO_FAILURE_STRING = "KERROR: IO Failure";

const char * strerror(int err_code);

#endif
