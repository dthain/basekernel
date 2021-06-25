/*
Copyright (C) 2016-2019 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/

#include "library/string.h"
#include "library/syscalls.h"
#include "library/stdio.h"
#include "library/nwindow.h"

#define SNAKE_MAX 128
#define THICK 5

struct coords { int x; int y; };

struct nwindow *nw = 0;

int randint(int min, int max)
{
	// Could be a lot better but at the moment it works
	uint32_t tm;
	syscall_system_time(&tm);
	int state = tm;
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
	int diff = max - min;

	return state % diff + min;
}

void init_snake_coords(struct coords *snake, int *length, int xsteps, int ysteps )
{
	*length = 3;

	snake[0].x = xsteps/2;
	snake[0].y = ysteps/2;

	int i;
	for(i=1;i<*length++;i++) {
		snake[i].x = snake[i-1].x-1;
		snake[i].y = snake[0].y;
	}

}

void scaled_rect( struct nwindow *nw, int x, int y, int scale )
{
	nw_rect(nw,x*scale,y*scale,scale,scale);
}

void update_snake(struct coords *snake, int *length, int x, int y, int grow )
{
	int i;

	if(!grow) {
		nw_fgcolor(nw,0,0,0);
		scaled_rect(nw,snake[*length-1].x,snake[*length-1].y,THICK);
	}

	*length += grow;

	for(i=*length-1;i>0;i--) {
		snake[i] = snake[i-1];
	}

	snake[0].x = x;
	snake[0].y = y;

	nw_fgcolor(nw,0,255,0);
	scaled_rect(nw,x,y,THICK);

}

int check_snake_collision( struct coords *snake, int length )
{
	int i;
	for(i=1;i<length;i++) {
		if(snake[i].x==snake[0].x && snake[i].y==snake[0].y) return 1;
	}
	return 0;
}

void init_apple( struct coords *apple, int xsteps, int ysteps )
{
	apple->x = randint(0,xsteps);
	apple->y = randint(0,ysteps);

	nw_fgcolor(nw,255, 0, 0);
	scaled_rect(nw,apple->x,apple->y,THICK);
}

int move_snake(struct coords *snake, int *length, struct coords *apple, int xsteps, int ysteps, char dir)
{
	int x, y;

	switch (dir) {
	case 'a':
		x = snake[0].x - 1;
		y = snake[0].y;
		break;
	case 'd':
		x = snake[0].x + 1;
		y = snake[0].y;
		break;
	case 'w':
		x = snake[0].x;
		y = snake[0].y - 1;
		break;
	case 's':
		x = snake[0].x;
		y = snake[0].y + 1;
		break;
	default:
		x = snake[0].x;
		y = snake[0].y;
		break;
	}

	if(x<=-1 || x>=xsteps || y<=-1 || y>=ysteps ) return -1;


	if(x==apple->x && y==apple->y) {
		update_snake(snake, length, x, y, 5);
		init_apple(apple,xsteps,ysteps);
	} else {
		update_snake(snake, length, x, y, 0);
	}

	if(check_snake_collision(snake,*length)) {
		return -1;
	}

	return 0;
}

int main(int argc, char *argv[])
{
	struct coords snake[SNAKE_MAX];
	struct coords apple;

	int snake_length = 3;
	int speed = 100;

	nw = nw_create_default();

	int width = nw_width(nw);
	int height = nw_height(nw);

	// Board dimensions in snake blocks
	int xsteps = width / THICK;
	int ysteps = height / THICK;

	nw_clear(nw,0, 0, width, height);
	nw_string(nw,THICK * 3, THICK * 4, "Press any key to start");
	nw_string(nw,THICK * 3, THICK * 8, "w: up");
	nw_string(nw,THICK * 3, THICK * 12, "s: down");
	nw_string(nw,THICK * 3, THICK * 16, "a: left");
	nw_string(nw,THICK * 3, THICK * 20, "d: right");
	nw_flush(nw);

	char c = nw_getchar(nw,1);


	while(1) {
		char dir = 'd';
		nw_clear(nw,0,0,width,height);
		init_snake_coords(snake, &snake_length, xsteps, ysteps);
		init_apple(&apple,xsteps, ysteps);

		while(1) {
			syscall_process_sleep(speed);
			c = nw_getchar(nw,0);

			if((c == 'a' && dir == 'd') || (c == 'd' && dir == 'a') || (c == 'w' && dir == 's') || (c == 's' && dir == 'w')) {
				// reject conflicting input
			} else if(c == 'w' || c == 'a' || c == 's' || c == 'd') {
				dir = c;
			} else if(c=='q') {
				break;
			}

			int status = move_snake(snake, &snake_length, &apple, xsteps, ysteps, dir);
			nw_flush(nw);
			if(status<0) break;
		}

		nw_flush(nw);
		nw_fgcolor(nw,255, 255, 255);
		nw_string(nw,THICK * 3, THICK * 4, "You lose!");
		nw_string(nw,THICK * 3, THICK * 8, "Enter q to quit");
		nw_string(nw,THICK * 3, THICK * 12, "Press any key to start");
		nw_flush(nw);

		c = nw_getchar(nw,1);
		if (c == 'q') return 0;
	}

}

