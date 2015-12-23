/*
Copyright (C) 2015 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/

#ifndef RTC_H
#define RTC_H

#include "kerneltypes.h"

struct rtc_time {
	uint8_t second;
	uint8_t minute;
	uint8_t hour;
	uint8_t day;
	uint8_t month;
	uint16_t year;
};

void rtc_init();
void rtc_read( struct rtc_time *t );

#endif
