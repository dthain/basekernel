/*
Copyright (C) 2018 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/


#include "library/syscalls.h"
#include "library/string.h"
#include "library/user-io.h"

/* 
	Display time in a window
*/


/* Function declarations */
void draw_border(int x, int y, int w, int h, int thickness, int r, int g, int b);
void draw_clock(uint32_t hour, uint32_t mins, int x, int y, int padding, int r, int g, int b, int tick);

int main(const char ** argv, int argc) 
{
	/* User needs to input initial time I guess */
	printf("%d\n", argc);
	uint32_t hour = (argv[1][0] - '0')*10 + (argv[1][1] - '0');
	uint32_t mins = (argv[1][3] - '0')*10 + (argv[1][4] - '0');

	/* Clock params */
	int CLOCK_W = 55;
	int CLOCK_H = 25;
	int thickness = 4;
	
	/* Set up window  */
	//int wd = open_window(KNO_STDWIN, 0, 0, CLOCK_W, CLOCK_H);
	draw_window(KNO_STDWIN);
	draw_clear(0, 0, CLOCK_W, CLOCK_H);
	draw_border(0, 0, CLOCK_W, CLOCK_H, thickness, 255, 255, 255);
	draw_clock(hour, mins, 0, 0, 2*thickness, 255, 255, 255, 1);
	draw_flush();

	/* Run Clock */
	char c = 0;
	int tick = 0;
	int seconds = 0;
	while(c != 'q')
	{
		read_nonblock(0, &c, 1);
		process_sleep(2000);
		seconds++;
		if (seconds == 60)
		{
			seconds = 0;
			mins++;
		}

		if (mins == 60)
		{
			mins = 0;
			hour++;
		}

		if (hour == 13)
		{
			hour = 1;
		}

		draw_clock(hour, mins, 0, 0, 2*thickness, 255, 255, 255, tick);
		tick = (tick == 0) ? 1:0;
		draw_flush();

	}


	return 0;
}

void draw_border(int x, int y, int w, int h, int thickness, int r, int g, int b) {
	draw_color(r, b, g);
	draw_rect(x, y, w, thickness);
	draw_rect(x, y, thickness, h);
	draw_rect(x + w - thickness, y, thickness, h);
	draw_rect(x, y + h - thickness, w, thickness);
}

void draw_clock(uint32_t hour, uint32_t mins, int x, int y, int padding, int r, int g, int b, int tick) {
	draw_color(r, b, g);

	char h_str[100];
	char m_str[100];


	uint_to_string(hour, h_str);
	uint_to_string(mins, m_str);

	char time[100] = "";

	if (strlen(h_str) == 1)
	{
		strcat(time, "0");
	}

	strcat(time, (const char *) h_str);
	if (tick) {
		strcat(time, ":");
	} else {
		strcat(time, " ");
	}
	
	strcat(time, (const char *) m_str);

	draw_string(x + padding, y + padding, time);
}
