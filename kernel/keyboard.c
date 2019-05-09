/*
Copyright (C) 2015-2019 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/

#include "console.h"
#include "ioports.h"
#include "interrupt.h"
#include "kernel/ascii.h"
#include "process.h"
#include "device.h"
#include "kernelcore.h"

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

static uint8_t buffer[BUFFER_SIZE];
static int keyboard_buffer_read = 0;
static int keyboard_buffer_write = 0;

static struct list queue = { 0, 0 };

static int shift_mode = 0;
static int alt_mode = 0;
static int ctrl_mode = 0;
static int capslock_mode = 0;
static int numlock_mode = 0;

static uint8_t keyboard_map( uint8_t code)
{
	int direction;

	if(code & 0x80) {
		direction = 0;
		code = code & 0x7f;
	} else {
		direction = 1;
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
	} else if(direction) {
		if(ctrl_mode && alt_mode && k->normal == ASCII_DEL) {
			reboot();
		} else if(capslock_mode) {
			if(k->special==KEYMAP_ALPHA && !shift_mode) {
				return k->shifted;
			} else {
				return k->normal;
			}	
		} else if(numlock_mode) {
			if(k->special==KEYMAP_NUMPAD && !shift_mode) {
				return k->shifted;
			} else {
				return k->normal;
			}	
		} else if(shift_mode) {
			return k->shifted;
		} else if(ctrl_mode) {
			return k->ctrled;
		} else {
			return k->normal;
		}
	}

	return KEY_INVALID;
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
				c = KEY_INVALID;
				break;
		}
	} else {
		c = keyboard_map(code);
	}

	if(c == KEY_INVALID) return;

	if((keyboard_buffer_write + 1) == (keyboard_buffer_read % BUFFER_SIZE))
		return;
	buffer[keyboard_buffer_write] = c;
	keyboard_buffer_write = (keyboard_buffer_write + 1) % BUFFER_SIZE;
	process_wakeup(&queue);
}

char keyboard_read( int non_blocking )
{
	while(keyboard_buffer_read == keyboard_buffer_write) {
		if(non_blocking) return -1;
		process_wait(&queue);
	}
	char c = buffer[keyboard_buffer_read];
	keyboard_buffer_read = (keyboard_buffer_read + 1) % BUFFER_SIZE;
	return c;
}

int keyboard_device_probe( int unit, int *nblocks, int *blocksize, char *name )
{
       if(unit==0) {
		strcpy(name,"keyboard");
		*nblocks = 0;
		*blocksize = 1;
		return 1;
       } else {
		return 0;
       }

}

int keyboard_device_read( int unit, void *data, int size, int offset)
{
	int i;
	char *cdata = data;
	for(i = 0; i < size; i++) {
		cdata[i] = keyboard_read(0);
	}
	return size;
}

int keyboard_device_read_nonblock( int unit, void *data, int size, int offset)
{
	int i;
	char *cdata = data;
	for(i = 0; i < size; i++) {
		cdata[i] = keyboard_read(1);
		if(cdata[i]==-1) return i;
	}
	return size;
}

static struct device_driver keyboard_driver = {
	.name          = "keyboard",
	.probe         = keyboard_device_probe,
	.read          = keyboard_device_read,
	.read_nonblock = keyboard_device_read_nonblock,
	0,
};

void keyboard_init()
{
	interrupt_register(33, keyboard_interrupt);
	interrupt_enable(33);
	device_driver_register(&keyboard_driver);
	printf("keyboard: ready\n");
}




