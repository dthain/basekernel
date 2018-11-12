#include "library/syscalls.h"
#include "library/string.h"
#include "library/user-io.h"

#define PADDING 4
#define MAX_SUB 6

/* Function Declarations */
int draw_border(int x, int y, int w, int h, int thickness, int r, int b, int g);
int create_subwindow(int subwins[MAX_SUB][4], int numpro, int w_r,int h_r, int w, int h);


int main(const char ** argv, int argc)
{
	int dims[2], numpro;
	get_dimensions(KNO_STDWIN, dims, 2);
	draw_window(KNO_STDWIN);
	draw_clear(0, 0, dims[0], dims[1]);
	draw_border(0, 0, dims[0], dims[1], 4, 255, 255, 255);
	draw_flush();

	str2int(argv[1], &numpro);

	//int col_width = dims[0]/numpro;
	//int x_init = 0;

	// int window_description[4];
	// window_description[0] = 0;
	// window_description[1] = 0;
	// window_description[2] = 300;
	// window_description[3] = 300;


	// process_wrun("bin/snake.exe", args, 1, window_description);
	const char *args[] = { "snake.exe" };
	process_run("snake.exe", args, 1);

	// for (int i = 0; i < numpro; i++)
	// {
	// 	draw_border(x_init, 0, col_width, dims[1], 4, 255, 255, 255);
	// 	x_init += col_width;
	// 	draw_flush();
	// }
	
	return 0;
}


int draw_border(int x, int y, int w, int h, int thickness, int r, int b, int g)
{
	// Color the border appropriately
	draw_color(r, b, g);

	// Draw 4 rectangles to represent the border
	draw_rect(x, y, w, thickness);
	draw_rect(x, y, thickness, h);
	draw_rect(w - thickness, y, thickness, h);
	draw_rect(x, h - thickness, w, 5);

	return 0;
}

int create_subwindow(int subwins[MAX_SUB][4], int win_num, int w_r,int h_r, int w, int h)
{
	return 0;
}
