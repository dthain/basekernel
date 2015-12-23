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

#define KEY_INVALID 0127

#define SPECIAL_SHIFT 1
#define SPECIAL_ALT   2
#define SPECIAL_CTRL  3
#define SPECIAL_SHIFTLOCK 4

#define KEYBOARD_PORT_A		0x60
#define KEYBOARD_PORT_B		0x61
#define KEYBOARD_ACK    0x80

#define BUFFER_SIZE 256

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
static int buffer_read = 0;
static int buffer_write = 0;

static struct list queue = {0,0};

static int keyboard_scan()
{
	int code;
	int ack;

	code = inb(KEYBOARD_PORT_A);	
	iowait();
	ack = inb(KEYBOARD_PORT_B);
	iowait();
	ack |= KEYBOARD_ACK;
	outb(ack,KEYBOARD_PORT_B);
	iowait();
	outb(ack,KEYBOARD_PORT_B);
	iowait();

	return code;
}

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

void keyboard_interrupt( int i, int code )
{
	char c;
	c = keyboard_map(keyboard_scan());
	if(c==KEY_INVALID) return;
	if((buffer_write+1) == (buffer_read%BUFFER_SIZE)) return;
	buffer[buffer_write] = c;
	buffer_write = (buffer_write+1)%BUFFER_SIZE;
	process_wakeup(&queue);
}

char keyboard_read()
{
	int result;
	while(buffer_read==buffer_write) {
		process_wait(&queue);
	}
	result = buffer[buffer_read];
	buffer_read = (buffer_read+1)%BUFFER_SIZE;
	return result;
}

void keyboard_init()
{
	interrupt_register(33,keyboard_interrupt);
	keyboard_scan();
	interrupt_enable(33);
	console_printf("keyboard: ready\n");
}

