/*
Copyright (C) 2018 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/


#include "library/syscalls.h"
#include "library/string.h"
#include "library/user-io.h"

/* 
	Testing the sys_process_wrun call
	This syscall should run a child process and set the childs
	Standard Window to a window the parent defines.
*/

int main(const char ** argv, int argc)
{
	/* Description of the Window the parent wants the child to have (x,y,w,h) */
	int window_description[4];
	window_description[0] = 0;
	window_description[1] = 0;
	window_description[2] = 300;
	window_description[3] = 300;

	const char *args[] = { "snake.exe" };
	process_wrun("snake.exe", args, 1, window_description, KNO_STDWIN);
	
	return 0;
}
