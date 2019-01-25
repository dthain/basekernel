/*
Copyright (C) 2015-2019 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/

#include "console.h"
#include "interrupt.h"
#include "clock.h"
#include "ioports.h"
#include "process.h"

#define CLICKS_PER_SECOND 10

#define TIMER0		0x40
#define TIMER_MODE	0x43
#define SQUARE_WAVE     0x36
#define TIMER_FREQ	1193182
#define TIMER_COUNT	(((unsigned)TIMER_FREQ)/CLICKS_PER_SECOND/2)

static uint32_t clicks = 0;
static uint32_t seconds = 0;

static struct list queue = { 0, 0 };

static void clock_interrupt(int i, int code)
{
	clicks++;
	process_wakeup_all(&queue);
	if(clicks >= CLICKS_PER_SECOND) {
		clicks = 0;
		seconds++;
		console_heartbeat(&console_root);
		process_preempt();
	}
}

clock_t clock_read()
{
	clock_t result;
	result.seconds = seconds;
	result.millis = 1000 * clicks / CLICKS_PER_SECOND;
	return result;
}

clock_t clock_diff(clock_t start, clock_t stop)
{
	clock_t result;
	if(stop.millis < start.millis) {
		stop.millis += 1000;
		stop.seconds -= 1;
	}
	result.seconds = stop.seconds - start.seconds;
	result.millis = stop.millis - start.millis;
	return result;
}

void clock_wait(uint32_t millis)
{
	clock_t start, elapsed;
	uint32_t total;

	start = clock_read();
	do {
		process_wait(&queue);
		elapsed = clock_diff(start, clock_read());
		total = elapsed.millis + elapsed.seconds * 1000;
	} while(total < millis);
}

void clock_init()
{
	outb(SQUARE_WAVE, TIMER_MODE);
	outb((TIMER_COUNT & 0xff), TIMER0);
	outb((TIMER_COUNT >> 8) & 0xff, TIMER0);

	interrupt_register(32, clock_interrupt);
	interrupt_enable(32);

	printf("clock: ticking\n");
}
