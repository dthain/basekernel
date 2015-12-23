/*
Copyright (C) 2015 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/

/*
Driver for the Motorola MC 146818A Real Time Clock
Recommended reading: page 11-15 of the RTC data sheet
*/

#include "kerneltypes.h"
#include "ioports.h"
#include "rtc.h"
#include "console.h"
#include "string.h"
#include "interrupt.h"

#define RTC_BASE 0x80

#define RTC_SECONDS        (RTC_BASE+0)
#define RTC_SECONDS_ALARM  (RTC_BASE+1)
#define RTC_MINUTES        (RTC_BASE+2)
#define RTC_MINUTES_ALARM  (RTC_BASE+3)
#define RTC_HOURS          (RTC_BASE+4)
#define RTC_HOURS_ALARM    (RTC_BASE+5)
#define RTC_DAY_OF_WEEK    (RTC_BASE+6)
#define RTC_DAY_OF_MONTH   (RTC_BASE+7)
#define RTC_MONTH          (RTC_BASE+8)
#define RTC_YEAR           (RTC_BASE+9)

#define RTC_REGISTER_A	   (RTC_BASE+10)
#define RTC_REGISTER_B     (RTC_BASE+11)
#define RTC_REGISTER_C     (RTC_BASE+12)
#define RTC_REGISTER_D     (RTC_BASE+13)

#define RTC_ADDRESS_PORT 0x70
#define RTC_DATA_PORT 0x71

/* Register A bits */

#define RTC_A_UIP (1<<7) 
#define RTC_A_DV2 (1<<6)
#define RTC_A_DV1 (1<<5)
#define RTC_A_DV0 (1<<4)
#define RTC_A_RS3 (1<<3)
#define RTC_A_RS2 (1<<2)
#define RTC_A_RS1 (1<<1)
#define RTC_A_RS0 (1<<0)

/* Register B bits */

#define RTC_B_SET  (1<<7) /* if set, may write new time */ 
#define RTC_B_PIE  (1<<6) /* periodic interrupt enabled */
#define RTC_B_AIE  (1<<5) /* alarm interrupt enabled */
#define RTC_B_UIE  (1<<4) /* update interrupt enabled */
#define RTC_B_SQWE (1<<3) /* square wave enabled */
#define RTC_B_DM   (1<<2) /* data mode: 1=binary 0=decimal */
#define RTC_B_2412 (1<<1) /* 1=24 hour mode 0=12 hour mode */
#define RTC_B_DSE  (1<<0) /* daylight savings enable */

/* Register C bits */
/* Note that reading C is necessary to acknowledge an interrupt */

#define RTC_C_IRQF (1<<7) /* 1=any interrupt pending */
#define RTC_C_PF   (1<<6) /* periodic interrupt pending */
#define RTC_C_AF   (1<<5) /* alarm interrupt pending */
#define RTC_C_UF   (1<<4) /* update interrupt pending */

static uint8_t rtc_bcd_to_binary( uint8_t bcd )
{
	return (bcd&0x0f) + (bcd>>4)*10;
}

static uint8_t rtc_read_port( uint16_t address )
{
	outb_slow(address,RTC_ADDRESS_PORT);
	return inb_slow(RTC_DATA_PORT);
}

static void rtc_write_port( uint8_t value, uint16_t address )
{
	outb_slow(address,RTC_ADDRESS_PORT);
	outb_slow(value,RTC_DATA_PORT);
}

static struct rtc_time cached_time;

static void rtc_fetch_time()
{
	struct rtc_time t;

	int addpm=0;

	do {
		t.second = rtc_read_port(RTC_SECONDS);
		t.minute = rtc_read_port(RTC_MINUTES);
		t.hour = rtc_read_port(RTC_HOURS);
		t.day = rtc_read_port(RTC_DAY_OF_MONTH);
		t.month = rtc_read_port(RTC_MONTH);
		t.year = rtc_read_port(RTC_YEAR);
	} while(t.second != rtc_read_port(RTC_SECONDS));

	if(t.hour&0x80) {
		addpm = 1;
		t.hour &= 0x7f;
	} else {
		addpm = 0;
	}

	t.second = rtc_bcd_to_binary(t.second);
	t.minute = rtc_bcd_to_binary(t.minute);
	t.hour   = rtc_bcd_to_binary(t.hour);
	if(addpm) t.hour += 12;
	t.day    = rtc_bcd_to_binary(t.day);
	t.month  = rtc_bcd_to_binary(t.month);
	t.year   = rtc_bcd_to_binary(t.year);

	if(t.year>=70) {
		t.year += 1900;
	} else {
		t.year += 2000;
	}

	cached_time = t;
}

static void rtc_interrupt_handler( int intr, int code )
{
	rtc_fetch_time();
	rtc_read_port(RTC_REGISTER_C);
}

void rtc_init()
{
	uint8_t status;

	rtc_fetch_time();

	status = rtc_read_port(RTC_REGISTER_B);
	status |= RTC_B_UIE;
	rtc_write_port(status,RTC_REGISTER_B);

	interrupt_register(40,rtc_interrupt_handler);
	interrupt_enable(40);

	console_printf("rtc: ready\n");
}

void rtc_read( struct rtc_time *tout )
{
	rtc_fetch_time();
	memcpy(tout,&cached_time,sizeof(cached_time));
}
