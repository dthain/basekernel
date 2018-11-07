#include "kernel/error.h"
#include "library/errno.h"
#include "library/string.h"

const char * strerror(int err_code) {
	if (err_code == KERROR_NOT_FOUND) {
  		return KERROR_NOT_FOUND_STRING;
	}
	if (err_code == KERROR_INVALID_REQUEST) {
  		return KERROR_INVALID_REQUEST_STRING;
	}
	if (err_code == KERROR_PERMISSION_DENIED) {
  		return KERROR_PERMISSION_DENIED_STRING;
	}
	if (err_code == KERROR_NOT_IMPLEMENTED) {
  		return KERROR_NOT_IMPLEMENTED_STRING;
	}
	if (err_code == KERROR_NOT_EXECUTABLE) {
  		return KERROR_NOT_EXECUTABLE_STRING;
	}
	if (err_code == KERROR_EXECUTION_FAILED) {
  		return KERROR_EXECUTION_FAILED_STRING;
	}
	if (err_code == KERROR_NOT_SUPPORTED) {
  		return KERROR_NOT_SUPPORTED_STRING;
	}
	if (err_code == KERROR_NOT_A_DIRECTORY) {
  		return KERROR_NOT_A_DIRECTORY_STRING;
	}
	if (err_code == KERROR_NOT_A_FILE) {
  		return KERROR_NOT_A_FILE_STRING;
	}
	if (err_code == KERROR_NOT_A_WINDOW) {
  		return KERROR_NOT_A_WINDOW_STRING;
	}
	if (err_code == KERROR_NOT_A_DEVICE) {
  		return KERROR_NOT_A_DEVICE_STRING;
	}
	if (err_code == KERROR_NO_MEMORY) {
  		return KERROR_NO_MEMORY_STRING;
	}
	if (err_code == KERROR_IO_FAILURE) {
  		return KERROR_IO_FAILURE_STRING;
	}
}
