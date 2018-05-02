/*
Copyright (C) 2017 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/

/*
A fun graphics demo that features text bouncing around the screen.
*/

#include "syscalls.h"
#include "string.h"
#include "user-io.h"

int sin_table[60] = {0, 105, 208, 309, 407, 500, 588, 669, 743, 809, 866, 914, 951, 978, 995, 1000, 995, 978, 951, 914, 866, 809, 743, 669, 588, 500, 407, 309, 208, 105, 0, -105, -208, -309, -407, -500, -588, -669, -743, -809, -866, -914, -951, -978, -995, -1000, -995, -978, -951, -914, -866, -809, -743, -669, -588, -500, -407, -309, -208, -105};

int WIDTH = (200);
int HEIGHT = (200);
int buff = 0;

typedef unsigned int uint32_t;

uint32_t randint(uint32_t min, uint32_t max);
void move(int *x, int *d, int min, int max);

void event_handler() {
    char s[12] = {0};
    int r = read(4, s, 12);
    int i = 0;
    for (i = 0; i < r; i++) {
        switch (s[i]) {
            case ',':
                buff = 0;
                break;
            case 'x':
                WIDTH = buff;
                buff = 0;
                break;
            case 'y':
                HEIGHT = buff;
                buff = 0;
                break;
            default:
                if (s[i] >= '0' && s[i] <= '9') {
                    buff = buff*10 + (s[i] - '0');
                }
                break;
        }
    }
}

//returns the sin of d (in degrees) from [-1000, 1000]
int sind(int d) {
    return sin_table[(d/6) % 60];
}

//returns the cos of d (in degrees) from [-1000, 1000]
int cosd(int d) {
    return sin_table[(d + 90)/6 % 60];
}

int main( const char *argv[], int argc )
{
    draw_window(KNO_STDWIN);
    draw_clear(0, 0, WIDTH, HEIGHT);
    draw_flush();

    for (;;) {
        int time = gettimeofday();
        int hour = (time / 60 / 60) % 12;
        int minute = (time / 60) % 60;
        int second = time % 60;
        
        draw_window(KNO_STDWIN);
        draw_clear(0, 0, WIDTH, HEIGHT);
        draw_color(255, 255, 255);
        draw_line(WIDTH/2, HEIGHT-10, 0, 10);
        draw_line(WIDTH/2, 0, 0, 10);
        draw_line(WIDTH-10, HEIGHT/2, 10, 0);
        draw_line(0, HEIGHT/2, 10, 0);

        //draw second hand
        int hsx = cosd(second * (360/60))*WIDTH/2000;
        int hsy = sind(second * (360/60))*HEIGHT/2000;
        draw_color(0, 0, 255);
        draw_line(WIDTH/2, HEIGHT/2, hsx, hsy);

        //draw minute hand
        int hmx = cosd(minute * (360/60))*WIDTH/3000;
        int hmy = sind(minute * (360/60))*HEIGHT/3000;
        draw_color(0, 255, 0);
        draw_line(WIDTH/2, HEIGHT/2, hmx, hmy);

        //draw hour hand
        int hhx = cosd(hour * (360/12))*WIDTH/4000;
        int hhy = sind(hour * (360/12))*HEIGHT/4000;
        draw_color(255, 0, 0);
        draw_line(WIDTH/2, HEIGHT/2, hhx, hhy);

        draw_flush();

        event_handler();
        sleep(75);
    }

	exit(0);

	return 0;
}
