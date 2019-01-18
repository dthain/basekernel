/*
Copyright (C) 2016-2019 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/

#include "mouse.h"
#include "console.h"
#include "ioports.h"
#include "interrupt.h"
#include "kernel/ascii.h"
#include "process.h"
#include "kernelcore.h"

/*
The PS2 interface uses a data port and a command port.
Reading the command port yields a status byte,
while writing to the command port executes commands.
*/

#define PS2_DATA_PORT    0x60
#define PS2_COMMAND_PORT 0x64

/* Some commands that may be sent to the command port. */

#define PS2_COMMAND_READ_CONFIG 0x20
#define PS2_COMMAND_WRITE_CONFIG 0x60
#define PS2_COMMAND_DISABLE_MOUSE 0xA7
#define PS2_COMMAND_ENABLE_MOUSE 0xA8
#define PS2_COMMAND_DISABLE_KEYBOARD 0xAD
#define PS2_COMMAND_ENABLE_KEYBOARD 0xAE
#define PS2_COMMAND_MOUSE_PREFIX 0xD4

/* The status byte read from the command port has these fields. */

#define PS2_STATUS_OBF 0x01	// true: may not write to data port
#define PS2_STATUS_IBF 0x02	// true: may read from data port
#define PS2_STATUS_SYS 0x04	// true when port is initialized
#define PS2_STATUS_A2  0x08	// true if command port was last written to
#define PS2_STATUS_INH 0x10	// true if keyboard inhibited
#define PS2_STATUS_MOBF 0x20	// true if mouse output available
#define PS2_STATUS_TOUT 0x40	// true if timeout during I/O
#define PS2_STATUS_PERR 0x80	// true indicates parity error

/*
In addition, a configuration byte may be read/written
via PS2_COMMAND_READ/WRITECONFIG.  The configuration byte
has these bitfields.
*/

#define PS2_CONFIG_PORTA_IRQ 0x01
#define PS2_CONFIG_PORTB_IRQ 0x02
#define PS2_CONFIG_SYSTEM    0x04
#define PS2_CONFIG_RESERVED1 0x08
#define PS2_CONFIG_PORTA_CLOCK 0x10
#define PS2_CONFIG_PORTB_CLOCK 0x20
#define PS2_CONFIG_PORTA_TRANSLATE 0x40
#define PS2_CONFIG_RESERVED2   0x80

/*
The mouse has several specialized commands that may
be sent by first sending PS2_COMMAND_MOUSE_PREFIX,
then one of the following:
*/

#define PS2_MOUSE_COMMAND_ENABLE_STREAMING 0xea
#define PS2_MOUSE_COMMAND_ENABLE_DEVICE    0xf4
#define PS2_MOUSE_COMMAND_RESET 0xff

/*
To read/write data to/from the PS2 port, we must first check
for the input/output buffer full (IBF/OBF) bits are set appropriately
in the status register.
*/

uint8_t ps2_read_data()
{
	uint8_t status;
	do {
		status = inb(PS2_COMMAND_PORT);
	} while(!(status & PS2_STATUS_OBF));
	return inb(PS2_DATA_PORT);
}

void ps2_write_data(uint8_t data)
{
	uint8_t status;
	do {
		status = inb(PS2_COMMAND_PORT);
	} while(status & PS2_STATUS_IBF);
	return outb(data, PS2_DATA_PORT);
}

/*
In a similar way, to write a command to the status port,
we must also check that the IBF field is cleared.
*/

void ps2_write_command(uint8_t data)
{
	uint8_t status;
	do {
		status = inb(PS2_COMMAND_PORT);
	} while(status & PS2_STATUS_IBF);
	return outb(data, PS2_COMMAND_PORT);
}

/*
Clear the buffer of all data by reading until OBF and IBF
are both clear.  Useful when resetting the device to achieve
a known state.
*/

void ps2_clear_buffer()
{
	uint8_t status;
	do {
		status = inb(PS2_COMMAND_PORT);
		if(status & PS2_STATUS_OBF) {
			inb(PS2_DATA_PORT);
			continue;
		}
	} while(status & (PS2_STATUS_OBF | PS2_STATUS_IBF));
}

/*
Send a mouse-specific command by sending the mouse prefix,
then the mouse command as data, then reading back an
acknowledgement.
*/

void ps2_mouse_command(uint8_t command)
{
	ps2_write_command(PS2_COMMAND_MOUSE_PREFIX);
	ps2_write_data(command);
	ps2_read_data();
}

/*
Read and write the PS2 configuration byte, which
does not involve an acknowledgement.
*/

uint8_t ps2_config_get()
{
	ps2_write_command(PS2_COMMAND_READ_CONFIG);
	return ps2_read_data();
}

void ps2_config_set(uint8_t config)
{
	ps2_write_command(PS2_COMMAND_WRITE_CONFIG);
	ps2_write_data(config);
}

static struct mouse_event state;

/*
On each interrupt, read three bytes from the PS 2 port, which
gives buttons and status, X and Y position.  The ninth (sign) bit
of the X and Y position is given as a single bit in the status
word, so we must assemble a twos-complement integer if needed.
Finally, take those values and update the current mouse state.
*/

static void mouse_interrupt(int i, int code)
{
	uint8_t m1 = inb(PS2_DATA_PORT);
	uint8_t m2 = inb(PS2_DATA_PORT);
	uint8_t m3 = inb(PS2_DATA_PORT);

	state.buttons = m1 & 0x03;
	state.x += m1 & 0x10 ? 0xffffff00 | m2 : m2;
	state.y -= m1 & 0x20 ? 0xffffff00 | m3 : m3;

	if(state.x < 0)
		state.x = 0;
	if(state.y < 0)
		state.y = 0;
	if(state.x >= video_xres)
		state.x = video_xres - 1;
	if(state.y >= video_yres)
		state.y = video_yres - 1;
}

/*
Do a non-blocking read of the current mouse state.
Block interrupts while reading, to avoid inconsistent state.
*/

void mouse_read(struct mouse_event *e)
{
	interrupt_disable(44);
	*e = state;
	interrupt_enable(44);
}

/*
Unlike the keyboard, the mouse is not automatically enabled
at bootup.  We must first obtain tbe ps2 configuration register,
enable both port A (keyboard) and port B (mouse), then send
a series of commands to reset the mouse and enable "streaming",
which causes an interrupt for every move of the mouse.
*/

void mouse_init()
{
	ps2_clear_buffer();

	uint8_t config = ps2_config_get();
	config |= PS2_CONFIG_PORTA_IRQ;
	config |= PS2_CONFIG_PORTB_IRQ;
	config &= ~PS2_CONFIG_PORTA_CLOCK;
	config &= ~PS2_CONFIG_PORTB_CLOCK;
	config |= PS2_CONFIG_PORTA_TRANSLATE;
	ps2_config_set(config);

	ps2_mouse_command(PS2_MOUSE_COMMAND_RESET);
	ps2_mouse_command(PS2_MOUSE_COMMAND_ENABLE_DEVICE);
	ps2_mouse_command(PS2_MOUSE_COMMAND_ENABLE_STREAMING);

	interrupt_register(44, mouse_interrupt);
	interrupt_enable(44);
	printf("mouse: ready\n");
}
