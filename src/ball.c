/*
Copyright (C) 2017 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/

/*
A fun graphics demo that features text bouncing around the screen.
*/

#include "syscalls.h"
#include "user-io.h"

int WIDTH = (200);
int HEIGHT = (200);
int buff = 0;

typedef unsigned int uint32_t;

uint32_t randint(uint32_t min, uint32_t max);
void move(int *x, int *d, int min, int max);

int main( const char *argv[], int argc )
{
    int r = 255;
    int g = 0;
    int b = 0;
    int x1 = 12;
    int y1 = 12;
    int dx1 = 4;
    int dy1 = 1;
    int dr = -1;
    int dg = 2;
    int db = 3;
    
    draw_window(KNO_STDWIN);
    draw_clear(0, 0, WIDTH, HEIGHT);
    draw_flush();

    for (;;) {
        draw_window(KNO_STDWIN);
        move(&x1, &dx1, 0, WIDTH-80);
        move(&y1, &dy1, 0, HEIGHT-1);
        move(&r, &dr, 0, 255);
        move(&g, &dg, 0, 255);
        move(&b, &db, 0, 255);
        draw_color(r, g, b);
        draw_string(x1, y1, "basekernel");
        draw_flush();

        char s[12] = {0};
        int r = read(4, s, 12);
        int i = 0;
        for (i = 0; i < r; i++) {
            switch (s[i]) {
                case ',':
                    buff = 0;
                    break;
                case 'x':
                    draw_clear(0, 0, WIDTH, HEIGHT);
                    WIDTH = buff;
                    buff = 0;
                    draw_clear(0, 0, WIDTH, HEIGHT);
                    break;
                case 'y':
                    draw_clear(0, 0, WIDTH, HEIGHT);
                    HEIGHT = buff;
                    buff = 0;
                    draw_clear(0, 0, WIDTH, HEIGHT);
                    break;
                default:
                    if (s[i] >= '0' && s[i] <= '9') {
                        buff = buff*10 + (s[i] - '0');
                    }
                    break;
            }
        }
        sleep(75);
    }

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
