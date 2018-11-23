/*
Copyright (C) 2018 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/


#include "library/syscalls.h"
#include "library/string.h"
#include "library/user-io.h"

/* 
	Have a list of programs and windows that we want to run in certain sized windows
	and place them across the screen
*/

#define num_programs 4

typedef struct program {
	int w;
	int h;
	const char * exec;
	const char ** args;
	int argc;
} program;

/* Function declarations */
void draw_border(int x, int y, int w, int h, int thickness, int r, int g, int b);
void mergeSort(program * programs, int l, int r);
void merge(program * arr, int l, int m, int r);


int main(const char ** argv, int argc) {
	/* Eventually, programs wont be hardcoded */
	const char *args1[] = { "/bin/snake.exe" };

	program programs[] = {
			{ .w = 400, .h = 300, .exec = "bin/snake.exe", .args = args1, .argc = 1 },
			{ .w = 330, .h = 320, .exec = "bin/snake.exe", .args = args1, .argc = 1 },
			{ .w = 305, .h = 290, .exec = "bin/snake.exe", .args = args1, .argc = 1 },
			{ .w = 308, .h = 300, .exec = "bin/snake.exe", .args = args1, .argc = 1 }
	};


	/* Setup the window */
	int std_dims[2];
	get_dimensions(KNO_STDWIN, std_dims, 2);
	draw_window(KNO_STDWIN);
	draw_clear(0, 0, std_dims[0], std_dims[1]);
	draw_flush();

	/* Sort programs in order of biggest height to smallest with smaller width being tie breaker */
	int left = 0;
	int right = num_programs-1;
	mergeSort(programs, left, right);

	/* Packing algorithm - First fit decreasing height - doesnt fit, skip it */
	int spacing = 6;
	int current_pos[num_programs][2] = { 0 }; // for each row, keep track of the current x position and height
	int placement[num_programs][3] = { 0 }; // (x, y, valid) of specific program
	int p_i, row;

	for (p_i = 0; p_i < num_programs; ++p_i) {
		for (row = 0; row < num_programs; ++row) {
			if (current_pos[row][0] + programs[p_i].w <= std_dims[0]) {
				// Looks like the program can be placed

				// If it is the first element in the row, x == 0
				// And, all of the rows havent been set, set the val of the next row
				if (current_pos[row][0] == 0) {
					// Probably need to keep track of y coords
					// If the program overlaps, we cant place it
					if (current_pos[row][1] + programs[p_i].h > std_dims[1]) {
						break;
					} else if (row < num_programs - 1) {
						// Otherwise, we can place that element in that row and we can
						// Set the y position of the next row
						current_pos[row+1][1] = current_pos[row][1] + spacing + programs[p_i].h;
					}
				}
				// Now, set the placement of this object
				placement[p_i][0] = current_pos[row][0]; // x coord
				placement[p_i][1] = current_pos[row][1]; // y coord
				placement[p_i][2] = 1; // program is validly placed
				current_pos[row][0] = current_pos[row][0] + spacing + programs[p_i].w;
				break;
			}
		}
	}

	/* Wrun each program */
	int pids[num_programs] = { 0 }, stdins[num_programs] = { 0 }, wd;
	for (p_i = 0; p_i < num_programs; ++p_i) {
		if (placement[p_i][2] == 0) {
			printf("INVALID\n");
			continue;
		}

		wd = open_window(KNO_STDWIN, placement[p_i][0], placement[p_i][1], programs[0].w, programs[0].h);
		stdins[p_i] = pipe_open();

		// Set pipe'd fd to be stdin for child
		pids[p_i] = process_wrun(programs[0].exec, programs[0].args, 1, wd, stdins[p_i]);
		close(wd); // Closed because manager doesnt want children to have access to others windows

		draw_window(KNO_STDWIN);
		draw_border(placement[p_i][0], placement[p_i][1], programs[p_i].w, programs[p_i].h, 4, 255, 255, 255);
		draw_flush();
	}

	/* Finally, allow the user to switch between programs*/
	int p_act = 0;
	char tin = 0;

	/* Draw green window around active process and start it */
	draw_window(KNO_STDWIN);
	draw_border(placement[p_act][0], placement[p_act][1], programs[p_act].w, programs[p_act].h, 4, 0, 0, 255);
	draw_flush();

	while (tin != 'q') {
		if (pids[p_act] == 0) {
			p_act = (p_act + 1) % num_programs;
			continue;
		}

		/* If tab entered, go to the next process */
		read(0, &tin, 1);
		if (tin == '\t') {
			draw_window(KNO_STDWIN);
			draw_border(placement[p_act][0], placement[p_act][1], programs[p_act].w, programs[p_act].h, 4, 255, 255, 255);
			draw_flush();
			p_act = (p_act + 1) % num_programs;
			/* Draw green window around active process and start it */
			draw_window(KNO_STDWIN);
			draw_border(placement[p_act][0], placement[p_act][1], programs[p_act].w, programs[p_act].h, 4, 0, 0, 255);
			draw_flush();
			continue;
		}
		/* Write 1 character to the correct pipe */
		write(stdins[p_act], &tin, 1);
	}

	/* Clean up the window */
	draw_color(255, 255, 255);
	draw_clear(0, 0, std_dims[0], std_dims[1]);
	draw_flush();
	return 0;
}


/** HELPER FUNCTIONS **/
// Code for mergesort taken from https://www.geeksforgeeks.org/merge-sort/
void mergeSort(program * arr, int l, int r) {
	if (l < r) {
		int m = l+(r-l)/2;
		mergeSort(arr, l, m);
		mergeSort(arr, m+1, r);
		merge(arr, l, m, r);
	}
}


void merge(program * arr, int l, int m, int r) {
	int i, j, k;
	int n1 = m - l + 1;
	int n2 = r - m;

	program L[n1], R[n2];

	for (i = 0; i < n1; ++i)
		L[i] = arr[l + i];
	for (j = 0; j < n2; ++j)
		R[j] = arr[m + 1 + j];

	i = 0;
	j = 0;
	k = l;

	while (i < n1 && j < n2) {
		if (L[i].h > R[j].h) {
			arr[k] = L[i];
			i++;
		} else if (R[j].h > L[i].h) {
			arr[k] = R[j];
			j++;
		} else if (L[i].w < R[j].w) {
			arr[k] = L[i];
			i++;
		} else {
			arr[k] = R[j];
			j++;
		}
		k++;
	}

	while (i < n1) {
		arr[k] = L[i];
		i++;
		k++;
	}

	while (j < n2) {
		arr[k] = R[j];
		j++;
		k++;
	}
}


void draw_border(int x, int y, int w, int h, int thickness, int r, int g, int b) {
	draw_color(r, b, g);
	draw_rect(x, y, w, thickness);
	draw_rect(x, y, thickness, h);
	draw_rect(x + w - thickness, y, thickness, h);
	draw_rect(x, y + h - thickness, w, thickness);
}
