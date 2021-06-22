/*
Copyright (C) 2016-2019 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/


#include "library/syscalls.h"
#include "library/string.h"
#include "library/stdio.h"
#include "library/nwindow.h"

/*
	Display time in a window
*/

void draw_clock( struct nwindow *nw, uint32_t hour, uint32_t minute, int timezone, int military, int x, int y, int padding, int r, int g, int b);

int main(int argc, char *argv[])
{
	int timezone = 5;
	int military = 0;

	/* Get initial time */
	struct rtc_time time;
	syscall_system_rtc(&time);

	/* Clock draw params */
	int CLOCK_W = 55;
	int CLOCK_H = 25;
	int thickness = 4;

	/* Set up window  */
	struct nwindow *nw = nw_create_default();

	/* Run Clock */
	while(nw_getchar(nw,0)!='q') 
	{
		syscall_system_rtc(&time);
		nw_clear(nw,0, 0, CLOCK_W, CLOCK_H);
		draw_clock(nw,time.hour, time.minute, timezone, military, 0, 0, 2*thickness, 255, 255, 255);
		syscall_process_sleep(2000);
	}

	return 0;
}

void draw_clock( struct nwindow *nw, uint32_t hour, uint32_t minute, int timezone, int military, int x, int y, int padding, int r, int g, int b)
{
	char h_str[100];
	char m_str[100];
	char time[100] = "";

	/* Configure time */
	int tz_hour = (int)hour - timezone;
	if (tz_hour < 0)
	{
		tz_hour += 24;
	}

	if (military == 0 && tz_hour > 12)
	{
		tz_hour -= 12;
	}

	uint_to_string((uint32_t) tz_hour, h_str);
	uint_to_string(minute, m_str);

	/* Format time string */
	if (strlen(h_str) == 1)
	{
		strcat(time, " ");
	}
	strcat(time, (const char *) h_str);
	strcat(time, ":");
	if (strlen(m_str) == 1)
	{
		strcat(time, "0");
	}
	strcat(time, (const char *) m_str);

	nw_fgcolor(nw,r,g,b);
	nw_string(nw,x + padding, y + padding, time);
	nw_flush(nw);
}
