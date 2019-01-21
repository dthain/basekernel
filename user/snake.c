/*
Copyright (C) 2016-2019 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/

/* Snake Game */
#include "library/string.h"

#include "library/syscalls.h"
#include "library/user-io.h"

typedef unsigned int uint32_t;


/* Keeps track of coordinates */
struct coords {
	uint8_t x_c;
	uint8_t y_c;
};

void test(struct coords *snake_coords, uint16_t x_steps, uint16_t y_steps, uint16_t x_start, uint16_t y_start);

/* Function declarations */
void init_snake_coords(struct coords *snake_coords, uint16_t x_steps, uint16_t y_steps, uint16_t x_start, uint16_t y_start);
uint8_t set_apple_location(uint16_t x_steps, uint16_t y_steps, struct coords *apple, uint8_t * board);
uint16_t randint(uint16_t min, uint16_t max);
int initialize_window(uint16_t x_b, uint16_t y_b, uint16_t w_b, uint16_t h_b, uint16_t thick, uint8_t r_b, uint8_t g_b, uint8_t b_b);
int draw_border(int x, int y, int w, int h, int thickness, int r, int g, int b);
void draw_board(uint16_t wd, uint16_t x_0, uint16_t y_0, uint16_t game_width, uint16_t game_height, uint16_t x_steps, uint16_t y_steps, struct coords *snake_coords, struct coords apple, uint16_t thick);
int move_snake(struct coords *snake_coords, struct coords *apple, uint16_t x_steps, uint16_t y_steps, uint8_t * board, char in);
int check_snake_collision(struct coords *snake_coords);
uint8_t check_snake_ate_apple(uint16_t x_next, uint16_t y_next, struct coords *apple);
void update_snake(struct coords *snake_coords, uint16_t x_next, uint16_t y_next, uint8_t grow);
void update_board(struct coords *snake_coords, uint8_t * board, uint16_t x_steps, uint16_t y_steps);

/* Main Execution */
int main(int argc, char *argv[])
{
	uint16_t WIDTH = 200;
	uint16_t HEIGHT = 200;
	uint16_t thick = 4;

	// Snake values
	uint16_t x = 0;
	uint16_t y = 0;

	// Board dimensions in pixels
	uint16_t game_width = WIDTH - thick * 2;
	uint16_t game_height = HEIGHT - thick * 2;

	// Board dimensions in snake blocks
	uint16_t x_steps = game_width / thick;
	uint16_t y_steps = game_height / thick;

	// Board keeps track of what cells a snake occupies
	uint8_t board[y_steps][x_steps];
	for(int i = 0; i < y_steps; i++) {
		for(int j = 0; j < x_steps; j++) {
			board[i][j] = 0;
		}
	}
	board[0][0] = 1;

	// Book keeping for snake and apple
	struct coords snake_coords[x_steps * y_steps];
	init_snake_coords(snake_coords, x_steps, y_steps, x, y);

	struct coords apple;
	if(set_apple_location(x_steps, y_steps, &apple, (uint8_t *) board) < 0) {
		printf("Setting apple location failed!\n");
		return 1;
	}
	// Misc. initializations
	int status, wd;
	char in;
	char tin;

	// Initialize the Window
	if((wd = initialize_window(x, y, WIDTH, HEIGHT, thick, 255, 255, 255)) < 0) {
		return 1;
	}
	draw_string(thick * 3, thick * 4, "Press any key to start");
	draw_string(thick * 3, thick * 8, "j: up");
	draw_string(thick * 3, thick * 12, "n: down");
	draw_string(thick * 3, thick * 16, "m: right");
	draw_string(thick * 3, thick * 20, "b: left");
	draw_flush();


	syscall_object_read(0, &tin, 1);
	if(tin != 'm' && tin != 'n') {
		in = 'm';
	} else {
		in = tin;
	}


	while(1) {
		// Draw the board
		draw_board(wd, thick, thick, game_width, game_height, x_steps, y_steps, snake_coords, apple, thick);

		// Wait
		syscall_process_sleep(100);

		// Get users next input -- non-blocking
		syscall_object_read_nonblock(0, &tin, 1);

		// Skip if the user goes reverse direction
		if((tin == 'b' && in == 'm') || (tin == 'm' && in == 'b') || (tin == 'j' && in == 'n') || (tin == 'n' && in == 'j'))
			in = in;
		else if(tin > 0 && (tin == 'b' || tin == 'm' || tin == 'n' || tin == 'j'))
			in = tin;

		// Try to move the snake
		status = move_snake(snake_coords, &apple, x_steps, y_steps, (uint8_t *) board, in);
		if(status == -1) {
			for(int i = 0; i < y_steps; i++) {
				for(int j = 0; j < x_steps; j++) {
					board[i][j] = 0;
				}
			}
			board[0][0] = 1;
			init_snake_coords(snake_coords, x_steps, y_steps, x, y);

			draw_flush();
			draw_color(255, 255, 255);
			draw_string(thick * 3, thick * 4, "You lose!");
			draw_string(thick * 3, thick * 8, "Enter q to quit");
			draw_string(thick * 3, thick * 12, "Press any key to start");
			draw_flush();
			syscall_object_read(0, &tin, 1);
			if (tin == 'q')
			{
				printf("Snake exiting\n");
				return 1;
			}
			if(tin != 'm' && tin != 'n') {
				tin = 'm';
			}
			in = tin;
			set_apple_location(x_steps, y_steps, &apple, (uint8_t *) board);
			draw_clear(0, 0, game_width, game_height);
			if(draw_border(0, 0, game_width, game_height, thick, 255, 255, 255) < 0) {
				printf("Border create failed!\n");
				return -1;
			}
		}
	}

}


void init_snake_coords(struct coords *snake_coords, uint16_t x_steps, uint16_t y_steps, uint16_t x_start, uint16_t y_start)
{
	for(uint16_t i = 0; i < x_steps * y_steps; i++) {
		snake_coords[i].x_c = 255;
		snake_coords[i].y_c = 255;
	}

	snake_coords[0].x_c = (uint8_t) x_start;
	snake_coords[0].y_c = (uint8_t) y_start;
}

uint8_t set_apple_location(uint16_t x_steps, uint16_t y_steps, struct coords *apple, uint8_t * board)
{
	// need a list of available coordinates
	// and then we will randomly pick from them
	struct coords possible_vals[x_steps * y_steps];
	init_snake_coords(possible_vals, x_steps, y_steps, 255, 255);	// none are possible to start

	// Keep track index values of possible_vals
	uint16_t curr_i = 0;

	// Add all potential coordinates to the random array
	for(uint16_t i = 0; i < y_steps; i++) {
		for(uint16_t j = 0; j < x_steps; j++) {
			if(*((board + i * y_steps) + j) == 0) {
				possible_vals[curr_i].x_c = (uint8_t) j;
				possible_vals[curr_i].y_c = (uint8_t) i;
				curr_i++;
			}
		}
	}

	// Get random int and use that to set the new value of apple
	int r = randint(0, curr_i);

	apple->x_c = possible_vals[r].x_c;
	apple->y_c = possible_vals[r].y_c;

	return 0;
}

uint16_t randint(uint16_t min, uint16_t max)
{
	// Could be a lot better but at the moment it works
	uint32_t tm;
	syscall_system_time(&tm);
	uint16_t state = tm;
	if(state % 5 == 0) {
		state += 50000;
	} else if(state % 4 == 0) {
		state += 1000;
	} else if(state % 3 == 0) {
		state += 10000;
	} else if(state % 2 == 0) {
		state += 25000;
	} else {
		state += 16000;
	}
	uint16_t diff = max - min;

	return state % diff + min;
}

int initialize_window(uint16_t x_b, uint16_t y_b, uint16_t w_b, uint16_t h_b, uint16_t thick, uint8_t r_b, uint8_t g_b, uint8_t b_b)
{
	/* draw initial window */
	draw_window(KNO_STDWIN);
	draw_clear(0, 0, w_b, h_b);
	if(draw_border(0, 0, w_b, h_b, thick, r_b, g_b, b_b) < 0) {
		printf("Border create failed!\n");
		return -1;
	}
	draw_flush();

	return KNO_STDWIN;
}

// Draws the border with the colors specified
int draw_border(int x, int y, int w, int h, int thickness, int r, int g, int b)
{
	// Color the border appropriately
	draw_color(r, b, g);

	// Draw 4 rectangles to represent the border
	draw_rect(x, y, w, thickness);
	draw_rect(x, y, thickness, h);
	draw_rect(x + w - thickness, y, thickness, h);
	draw_rect(x, y + h - thickness, w, thickness);

	return 0;
}

void draw_board(uint16_t wd, uint16_t x_0, uint16_t y_0, uint16_t game_width, uint16_t game_height, uint16_t x_steps, uint16_t y_steps, struct coords *snake_coords, struct coords apple, uint16_t thick)
{
	draw_clear(x_0, y_0, game_width, game_height);
	draw_window(wd);
	// Draw the snake
	draw_color(0, 255, 0);
	for(uint16_t i = 0; i < x_steps * y_steps; i++) {
		if(snake_coords[i].x_c == 255) {
			break;
		}
		draw_rect(snake_coords[i].x_c * thick + x_0, snake_coords[i].y_c * thick + y_0, thick, thick);
	}

	// Draw the apple
	draw_color(255, 0, 0);
	draw_rect(apple.x_c * thick + x_0, apple.y_c * thick + y_0, thick, thick);

	draw_flush();
}

int move_snake(struct coords *snake_coords, struct coords *apple, uint16_t x_steps, uint16_t y_steps, uint8_t * board, char in)
{
	// Set snakes next coordinates
	uint16_t x_next, y_next;
	switch (in) {
	case ('b'):
		if(snake_coords[0].x_c == 0)
			return -1;
		x_next = snake_coords[0].x_c - 1;
		y_next = snake_coords[0].y_c;
		break;
	case ('m'):
		if(snake_coords[0].x_c == x_steps - 1)
			return -1;
		x_next = snake_coords[0].x_c + 1;
		y_next = snake_coords[0].y_c;
		break;
	case ('j'):
		if(snake_coords[0].y_c == 0)
			return -1;
		x_next = snake_coords[0].x_c;
		y_next = snake_coords[0].y_c - 1;
		break;
	case ('n'):
		if(snake_coords[0].y_c == y_steps - 1)
			return -1;
		x_next = snake_coords[0].x_c;
		y_next = snake_coords[0].y_c + 1;
		break;
	default:
		return 0;
	}

	// Check if snake needs to grow
	if(check_snake_ate_apple(x_next, y_next, apple)) {
		// Update snake_coords with an extra snake block
		update_snake(snake_coords, x_next, y_next, 1);

		// Update the board
		update_board(snake_coords, board, x_steps, y_steps);

		// Relocate the apple
		if(set_apple_location(x_steps, y_steps, apple, board) < 0) {
			printf("Setting apple location failed!\n");
			return -1;
		}
	} else {
		// Update snake_coords
		update_snake(snake_coords, x_next, y_next, 0);

		// Update the board
		update_board(snake_coords, board, x_steps, y_steps);
	}

	// Check if snake head collided with any other values
	if(check_snake_collision(snake_coords)) {
		return -1;
	}

	return 0;
}

int check_snake_collision(struct coords *snake_coords)
{
	uint16_t x_0 = snake_coords[0].x_c;
	uint16_t y_0 = snake_coords[0].y_c;

	uint16_t s_i = 1;

	while(snake_coords[s_i].x_c != 255) {
		if(x_0 == snake_coords[s_i].x_c && y_0 == snake_coords[s_i].y_c) {
			return -1;
		}
		s_i++;
	}

	return 0;
}

uint8_t check_snake_ate_apple(uint16_t x_next, uint16_t y_next, struct coords * apple)
{
	if(x_next == apple->x_c && y_next == apple->y_c) {
		return 1;
	}
	return 0;
}

void update_snake(struct coords *snake_coords, uint16_t x_next, uint16_t y_next, uint8_t grow)
{

	uint8_t x_tmp, y_tmp;
	uint16_t curr_i = 0;

	while(snake_coords[curr_i].x_c != 255) {
		x_tmp = snake_coords[curr_i].x_c;
		y_tmp = snake_coords[curr_i].y_c;

		snake_coords[curr_i].x_c = (uint8_t) x_next;
		snake_coords[curr_i].y_c = (uint8_t) y_next;

		x_next = (uint16_t) x_tmp;
		y_next = (uint16_t) y_tmp;

		curr_i++;
	}

	// Chop off the last block if need be
	if(grow) {
		snake_coords[curr_i].x_c = x_next;
		snake_coords[curr_i].y_c = y_next;
	}
}

void update_board(struct coords *snake_coords, uint8_t * board, uint16_t x_steps, uint16_t y_steps)
{
	// Zero out board
	for(uint16_t i = 0; i < y_steps; i++) {
		for(uint16_t j = 0; j < x_steps; j++) {
			(board + i * y_steps)[j] = 0;
		}
	}

	// Add values with values of snake
	uint16_t curr_i = 0;
	while(snake_coords[curr_i].x_c != 255) {
		(board + snake_coords[curr_i].y_c * y_steps)[snake_coords[curr_i].x_c] = 1;
		curr_i++;
	}
}
