/*
Copyright (C) 2015-2019 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/

#include "console.h"
#include "ioports.h"
#include "interrupt.h"
#include "kernel/ascii.h"
#include "kernelcore.h"
#include "event_queue.h"

#define KEYBOARD_PORT 0x60

#define KEYMAP_SHIFT 1
#define KEYMAP_ALT   2
#define KEYMAP_CTRL  3
#define KEYMAP_CAPSLOCK 4
#define KEYMAP_NUMLOCK 5
#define KEYMAP_ALPHA 6
#define KEYMAP_NUMPAD 8

/* sent before certain keys such as up, down, left, or right. */
#define KEYCODE_EXTRA (uint8_t)0xE0
#define KEYCODE_UP    (uint8_t)0x48
#define KEYCODE_DOWN  (uint8_t)0x42
#define KEYCODE_LEFT  (uint8_t)0x4B
#define KEYCODE_RIGHT (uint8_t)0x4D

#define BUFFER_SIZE 256

struct keymap {
	char normal;
	char shifted;
	char ctrled;
	char special;
};

static struct keymap keymap[] = {
#include "keymap.us.pc.c"
};

static int shift_mode = 0;
static int alt_mode = 0;
static int ctrl_mode = 0;
static int capslock_mode = 0;
static int numlock_mode = 0;

static void keyboard_interrupt_l2( uint8_t code )
{
	int direction;
	int event;

	if(code & 0x80) {
		direction = 0;
		event = EVENT_KEY_UP;
		code = code & 0x7f;
	} else {
		direction = 1;
		event = EVENT_KEY_DOWN;
	}

	struct keymap *k = &keymap[code];

	if(k->special == KEYMAP_SHIFT) {
		shift_mode = direction;
	} else if(k->special == KEYMAP_ALT) {
		alt_mode = direction;
	} else if(k->special == KEYMAP_CTRL) {
		ctrl_mode = direction;
	} else if(k->special == KEYMAP_CAPSLOCK) {
		if(direction == 0) capslock_mode = !capslock_mode;
	} else if(k->special == KEYMAP_NUMLOCK) {
		if(direction == 0) numlock_mode = !numlock_mode;
	} else {
		if(direction && ctrl_mode && alt_mode && k->normal == ASCII_DEL) {
			reboot();
		} else if(capslock_mode) {
			if(k->special==KEYMAP_ALPHA && !shift_mode) {
				event_queue_post_root(event,k->shifted,0,0);
			} else {
				event_queue_post_root(event,k->normal,0,0);
			}	
		} else if(numlock_mode) {
			if(k->special==KEYMAP_NUMPAD && !shift_mode) {
				event_queue_post_root(event,k->shifted,0,0);
			} else {
				event_queue_post_root(event,k->normal,0,0);
			}	
		} else if(shift_mode) {
			event_queue_post_root(event,k->shifted,0,0);
		} else if(ctrl_mode) {
			event_queue_post_root(event,k->ctrled,0,0);
		} else {
			event_queue_post_root(event,k->normal,0,0);
		}
	}
}

static int expect_extra = 0;

static void keyboard_interrupt(int i, int intr_code)
{
	uint8_t code = inb(KEYBOARD_PORT);
	uint8_t c = KEY_INVALID;

	if(code == KEYCODE_EXTRA) {
		expect_extra = 1;
		return;
	} else if(expect_extra) {
		expect_extra = 0;
		switch(code) {
			case KEYCODE_UP:
				c = KEY_UP;
				break;
			case KEYCODE_DOWN:
				c = KEY_DOWN;
				break;
			case KEYCODE_LEFT:
				c = KEY_LEFT;
				break;
			case KEYCODE_RIGHT:
				c = KEY_RIGHT;
				break;
			default:
				return;
		}
	} else {
		c = code;
	}

	keyboard_interrupt_l2(c);
}

void keyboard_init()
{
	interrupt_register(33, keyboard_interrupt);
	interrupt_enable(33);
	printf("keyboard: ready\n");
}




