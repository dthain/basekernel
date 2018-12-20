#ifndef ERRNO_H
#define ERRNO_H

#include "kernel/error.h"

const char *strerror( kernel_error_t err_code);

#endif
