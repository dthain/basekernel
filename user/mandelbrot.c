/*
Copyright (C) 2016-2019 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/


#include "library/syscalls.h"
#include "library/string.h"
#include "library/user-io.h"

#define ITER_THRES 100
#define VAL_THRES 2
#define STEPS 5

/*
	Compute and graph the mandelbrot set
*/

typedef struct Complex {
	float r;
	float i;
} Complex;

int in_set(Complex c);
void plot_point(int iter_val, int j, int k);

int main(int argc, char *argv[])
{
	/* Set up params */
	int dim = 400;
	float r_init = 0.3;
	float i_init = -0.19;
	float factor = 0.001;

	/* Setup the window */
	draw_window(KNO_STDWIN);
	draw_clear(0, 0, dim, dim);

	/* For each point, see if it is in the Mandelbrot set */
	Complex current;
	int iter_val, j, k;

	for (j = 0; j < dim; ++j)
	{
		for (k = 0; k < dim; ++k)
		{
			current.r = (float)j*factor + r_init;
			current.i = (float)k*factor + i_init;
			iter_val = in_set(current);
			plot_point(iter_val, j, k);
			draw_flush();
		}
		syscall_process_yield();
	}

	return 0;
}

int in_set(Complex c)
{
	/*
		Checks if the number is in the set
		Returns iteration it exceeded the threshold
		If it does not, return one greater than Threshold
	*/
	Complex z = { .r = 0, .i = 0};
	int i = 0;
	for (i = 0; i < ITER_THRES; ++i)
	{
		z.r = z.r*z.r + -1*z.i*z.i + c.r;
		z.i = 2*z.r*z.i + c.i;

		if (z.r > VAL_THRES) {
			return i;
		}
	}

	return ITER_THRES + 1;
}

void plot_point(int iter_val, int j, int k)
{
	/* Plot the point based on the color */
	int step_size = ITER_THRES/STEPS;
	int colors[STEPS][3] = {{255,255,0},{255,215,0},{50,205,50},{0,0,205},{165,42,42}};

	if (iter_val > ITER_THRES) {
		draw_color(255, 255, 255);
		draw_rect(j, k, 1, 1);
	} else {
		for (int i = 0; i < STEPS; ++i)
		{
			if (iter_val <= step_size*(i+1)) {
				draw_color(colors[i][0], colors[i][1], colors[i][2]);
				draw_rect(j, k, 1, 1);
				break;
			}
		}
	}
}
