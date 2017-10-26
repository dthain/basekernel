/*
Copyright (C) 2017 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/

/*
A fun graphics demo that features a line segment bouncing around the screen.
*/

#include "syscalls.h"
#define WIDTH    (200)
#define HEIGHT   (300)
typedef unsigned int uint32_t;

uint32_t randint(uint32_t min, uint32_t max);
void move(int *x, int *d, int min, int max);

int main( const char *argv[], int argc )
{
    int wd = draw_create(0, 100, 100, WIDTH, HEIGHT);
    if (wd < 0) {
        debug("Window create failed!\n");
        exit(1);
    }

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
    
    draw_clear(wd, 0, 0, WIDTH, HEIGHT);

    int i, j;
    for (;;) {
        move(&x1, &dx1, 0, WIDTH-1);
        move(&y1, &dy1, 0, HEIGHT-1);
        move(&x2, &dx2, 0, WIDTH-1);
        move(&y2, &dy2, 0, HEIGHT-1);
        move(&r, &dr, 0, 255);
        move(&g, &dg, 0, 255);
        move(&b, &db, 0, 255);
        draw_color(wd, r, g, b);

        draw_line(wd, x1, y1, x2-x1, y2-y1);
        yield();
    }

    draw_clear(wd, 0, 0, WIDTH+1, HEIGHT+1);
	exit(0);

	return 0;
}

uint32_t randint(uint32_t min, uint32_t max) {
    static uint32_t state = 0xF3DC1A24;
    state = (state * 1299721) + 29443;
    return min + ((state>>16) % (max - min + 1));
}

void move(int *x, int *d, int min, int max) {
    *x += *d;
    if (*x < min) {
        *x = min;
        *d = randint(1, 10);
    }
    if (*x > max) {
        *x = max;
        *d = -randint(1, 10);
    }
}
