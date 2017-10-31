/*
Copyright (C) 2017 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/

/*
A trivial user level program to try out basic system calls.
This program requires that write() and exit() work correctly.
*/

#include "syscalls.h"
#include "string.h"
void a(){}
int main( const char *argv[], int argc )
{

    int wd = draw_create(0, 100, 100, 300, 300);
    printf("hello world, I am %d!\n", getpid());
	exit(0);
  
	return 0;
}
