/*
Copyright (C) 2016-2019 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/


#include "library/syscalls.h"
#include "library/string.h"
#include "library/user-io.h"

#define MAX_ITERS 2000

int in_set( float x, float y );
void plot_point(int iter, int j, int k);

int main(int argc, char *argv[])
{
	/* measure size of window */
        int size[2];
        syscall_object_size(KNO_STDWIN, size, 2);

	int xsize = size[0];
	int ysize = size[1];

	float xlow = -2.0;
	float ylow = -1.5;
	float xfactor = 2.5/xsize;
	float yfactor = 3.0/ysize;

	/* Setup the window */
	draw_window(KNO_STDWIN);
	draw_clear(0, 0,xsize,ysize);

	/* For each point, see if it is in the Mandelbrot set */
	int iter,i,j;

	for(i=0;i<xsize;i++) {
		for(j=0;j<ysize;j++) {
			float x = i*xfactor + xlow;
			float y = j*yfactor + ylow;
			iter = in_set(x,y);
			plot_point(iter,i,j);
			draw_flush();
		}
		syscall_process_yield();
	}

	return 0;
}

int in_set( float x0, float y0 )
{
	int i=0;
	float x=x0;
	float y=y0;

	while( x*x + y*y < 4 && i<MAX_ITERS) {
		float t = x*x - y*y + x0;
		y = 2*x*y + y0;
		x = t;
		i++;
	}
	return i;
}

void plot_point(int iter, int j, int k)
{
	if(iter==MAX_ITERS) {
		draw_fgcolor(0,0,0);
	} else {
		int color[3];
		color[0] = iter   % 256;
		color[1] = iter*3 % 256;
		color[2] = iter*7 % 256;
		draw_fgcolor(color[0],color[1],color[2]);
	}
	draw_rect(j,k,1,1);
}
