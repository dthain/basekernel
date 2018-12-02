/*
Copyright (C) 2018 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/


#include "library/syscalls.h"
#include "library/string.h"
#include "library/user-io.h"

#define ITER_THRES 40000000
#define VAL_THRES 2
#define STEPS 5

/* 
	Compute and graph the mandelbrot set
*/

typedef struct Complex {
	int r;
	int i;
} Complex;

int in_set(Complex c);
void draw_border(int x, int y, int w, int h, int thickness, int r, int g, int b);
void plot_point(Complex c, int iter_val, int origin);

int main(const char ** argv, int argc) 
{
	/* Set up params */
	int dim = 400;
	int orgin = dim/2;

	/* Setup the window */
	int wd = open_window(KNO_STDWIN, 0, 0, dim, dim);
	draw_window(wd);
	draw_clear(0, 0, dim, dim);

	/* For each point, see if it is in the Mandelbrot set */
	Complex current;
	int iter_val, i, j, k;
	for (i = 0; i < 4; ++i)
	{
		for (j = 0; j < orgin; ++j)
		{
			for (k = 0; k < orgin; ++k)
			{
				process_sleep(100);
				if (i == 0) {
					current.r = j;
					current.i = k;
				} else if (i == 1) {
					current.r = -j;
					current.i = k;
				} else if (i == 2) {
					current.r = -j;
					current.i = -k;
				} else {
					current.r = j;
					current.i = -k;
				}
				iter_val = in_set(current);
				plot_point(current, iter_val, orgin);
				draw_window(wd);
				draw_flush();
			}
		}

	}

	return 0;
}

void draw_border(int x, int y, int w, int h, int thickness, int r, int g, int b) 
{
	draw_color(r, b, g);
	draw_rect(x, y, w, thickness);
	draw_rect(x, y, thickness, h);
	draw_rect(x + w - thickness, y, thickness, h);
	draw_rect(x, y + h - thickness, w, thickness);
}

int in_set(Complex c)
{
	Complex z = { .r = 0, .i = 0};
	int i = 0;
	for (i = 0; i < ITER_THRES; ++i)
	{
		z.r = z.r*z.r + -1*z.i*z.i + c.r;
		z.i = 2*z.r*z.i + c.i;

		if (z.r > VAL_THRES) {
			return ITER_THRES + 1;
		}
	}

	return i;
}

void plot_point(Complex c, int iter_val, int orgin)
{
	int step_size = ITER_THRES/STEPS;
	int colors[STEPS][3] = {{255,255,0},{255,215,0},{50,205,50},{0,0,205},{165,42,42}};

	if (iter_val > ITER_THRES) {
		draw_color(255, 255, 255);
		draw_rect(c.r + orgin, c.i + orgin, 1, 1);
	} else {
		for (int i = 0; i < STEPS; ++i)
		{
			if (iter_val <= step_size*(i+1)) {
				draw_color(colors[i][0], colors[i][1], colors[i][2]);
				draw_rect(c.r + orgin, c.i + orgin, 1, 1);
				break;
			}
		}
	}
}
