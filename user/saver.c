/*
Copyright (C) 2016-2019 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/

/*
A fun graphics demo that features a line segment bouncing around the screen.
*/

#include "library/syscalls.h"
#include "library/stdio.h"
#include "library/string.h"
#include "library/nwindow.h"

typedef unsigned int uint32_t;

uint32_t randint(uint32_t min, uint32_t max);
void move(int *x, int *d, int min, int max);

struct nwindow *nw = 0;

int main(int argc, char *argv[])
{
	int r = 255;
	int g = 0;
	int b = 0;
	int x1 = 12;
	int y1 = 12;
	int x2 = 48;
	int y2 = 48;
	int dx1 = 4;
	int dy1 = 1;
	int dx2 = -2;
	int dy2 = -2;
	int dr = -1;
	int dg = 2;
	int db = 3;

	nw = nw_create_default();

	int width = nw_width(nw);
	int height = nw_height(nw);

	nw_clear(nw,0, 0, width, height);
	nw_flush(nw);

	while(nw_getchar(nw,0)!='q') {
		move(&x1, &dx1, 0, width - 1);
		move(&y1, &dy1, 0, height - 1);
		move(&x2, &dx2, 0, width - 1);
		move(&y2, &dy2, 0, height - 1);
		move(&r, &dr, 0, 255);
		move(&g, &dg, 0, 255);
		move(&b, &db, 0, 255);
		nw_fgcolor(nw,r, g, b);
		nw_line(nw,x1, y1, x2 - x1, y2 - y1);
		nw_flush(nw);

		syscall_process_sleep(2);
	}
	nw_clear(nw,0, 0, width, height);
	nw_fgcolor(nw,255, 255, 255);
	nw_flush(nw);
	return 0;
}

uint32_t randint(uint32_t min, uint32_t max)
{
	static uint32_t state = 0xF3DC1A24;
	state = (state * 1299721) + 29443;
	return min + ((state >> 16) % (max - min + 1));
}

void move(int *x, int *d, int min, int max)
{
	*x += *d;
	if(*x < min) {
		*x = min;
		*d = randint(1, 10);
	}
	if(*x > max) {
		*x = max;
		*d = -randint(1, 10);
	}
}
