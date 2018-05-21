/*
Copyright (C) 2015 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/

#include "console.h"
#include "ioports.h"
#include "interrupt.h"
#include "ascii.h"
#include "process.h"
#include "kernelcore.h"
#include "device.h"

#define KEYBOARD_PORT 0x60

#define KEY_INVALID 127
#define KEY_EXTRA   -32 /*sent before certain keys such as up, down, left, or right(*/

#define SPECIAL_SHIFT 1
#define SPECIAL_ALT   2
#define SPECIAL_CTRL  3
#define SPECIAL_SHIFTLOCK 4

#define BUFFER_SIZE 256

struct device keyboard = {0};

struct keymap {
	char	normal;
	char	shifted;
	char	ctrled;
	char	special;
};
static struct keymap keymap[] = {
#include "keymap.us.c"
};

static char buffer[BUFFER_SIZE];
static int keyboard_buffer_read = 0;
static int keyboard_buffer_write = 0;

static struct list queue = {0,0};

static int shift_mode = 0;
static int alt_mode = 0;
static int ctrl_mode = 0;
static int shiftlock_mode = 0;

static char keyboard_map( int code )
{
	int direction;

	if(code&0x80) {
		direction = 0;
		code = code&0x7f;
	} else {
		direction = 1;
	}

	if(keymap[code].special==SPECIAL_SHIFT) {
		shift_mode = direction;		
		return KEY_INVALID;
	} else if(keymap[code].special==SPECIAL_ALT) {
		alt_mode = direction;
		return KEY_INVALID;
	} else if(keymap[code].special==SPECIAL_CTRL) {
		ctrl_mode = direction;
		return KEY_INVALID;
	} else if(keymap[code].special==SPECIAL_SHIFTLOCK) {
		if(direction==0) shiftlock_mode = !shiftlock_mode;
		return KEY_INVALID;
	} else if(direction) {
		if(ctrl_mode && alt_mode && keymap[code].normal==ASCII_DEL) {
			reboot();
			return KEY_INVALID;
		} else if(shiftlock_mode) {
			if(shift_mode) {
				return keymap[code].normal;
			} else {
				return keymap[code].shifted;
			}
		} else if(shift_mode) {
			return keymap[code].shifted;		
		} else if(ctrl_mode) {
			return keymap[code].ctrled;		
		} else {
			return keymap[code].normal;
		}
	} else {
		return KEY_INVALID;
	}
}

static void keyboard_interrupt( int i, int code)
{
    static char mod = 0x00;
    char c = inb(KEYBOARD_PORT);
    if (c == KEY_EXTRA) {
        mod = 0x80;
        return;
    } else {
        c = keyboard_map(c) | mod;
        mod = 0x00;
    }
	if(c==KEY_INVALID) return;
	if((keyboard_buffer_write+1) == (keyboard_buffer_read%BUFFER_SIZE)) return;
	buffer[keyboard_buffer_write] = c;
	keyboard_buffer_write = (keyboard_buffer_write+1)%BUFFER_SIZE;
	process_wakeup(&queue);
}

int keyboard_device_read(struct device* d, void* dest, int size, int offset)
{
	int i;
	for (i = 0; i < size; i++) {
		while(keyboard_buffer_read==keyboard_buffer_write) {
			process_wait(&queue);
		}
		((char*)dest)[i] = buffer[keyboard_buffer_read];
		keyboard_buffer_read = (keyboard_buffer_read+1)%BUFFER_SIZE;
	}
	return size;
}

char keyboard_read()
{
	char toRet = 0;
	device_read(&keyboard, &toRet, 1, 0);
	return toRet;
}

struct device* keyboard_get()
{
    return &keyboard;
}

void keyboard_init()
{
    keyboard.block_size = 1;
    keyboard.read = keyboard_device_read;
	interrupt_register(33,keyboard_interrupt);
	interrupt_enable(33);
	console_printf("keyboard: ready\n");
}

