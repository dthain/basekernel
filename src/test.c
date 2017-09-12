/*
A trivial user level program to try out basic system calls.
This program requires that write() and exit() work correctly.
*/

int main( const char *argv[], int argc );

/*
The very first element in the text portion of the program
must be the start function b/c the kernel will jump to virtual
memory location zero.  This function cannot return, but must
invoke the exit() system call in order to stop properly. 
*/

void _start() {
	main(0,0);
}

#include "syscall.h"

int main( const char *argv[], int argc )
{
	int i, j;

	for(j=0;j<10;j++) {
		write(1,"hello world!\n",13);
     		for(i=0;i<100000000;i++)  {}
	}

	exit(0);

	return 0;
}
