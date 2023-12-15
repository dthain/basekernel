
#include "library/stdlib.h"
#include "library/syscalls.h"

void exit( int code )
{
	syscall_process_exit(code);
	
}

/* Some easy things yet to be implemented. */

int rand();
int srand( int );
int system( const char *str );
int sleep( int seconds );
int usleep( int usec );
int time( int *t );
struct timeval;
int gettimeofday( struct timeval *tval, void *arg );
