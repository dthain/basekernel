#ifndef SYSCALL_HANDLER_H
#define SYSCALL_HANDLER_H

/* Only syscall handlers invoked by other parts of kernel code should be declared here. */

int sys_process_run( const char *path, const char **argv, int argc );

#endif

