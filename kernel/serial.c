/*
Copyright (C) 2016-2019 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/

// Derived from: https://wiki.osdev.org/Serial_Ports#Example_Code
// For more info:
//     https://wiki.osdev.org/Serial_Ports
//     http://www.webcitation.org/5ugQv5JOw

#include "kernel/types.h"
#include "ioports.h"
#include "string.h"
#include "device.h"

#define COM1 0x3f8
#define COM2 0x2F8
#define COM3 0x3E8
#define COM4 0x2E8

#define SERIAL_DATA 0		// If DLAB disabled in LCR

#define SERIAL_IRQ_ENABLE 1	// If DLAB disabled in LCR
#define SERIAL_IRQ_DATA_AVAILABILE (0x01 << 0)
#define SERIAL_IRQ_TRASMITTER_EMPTY (0x01 << 1)
#define SERIAL_IRQ_ERROR (0x01 << 2)
#define SERIAL_IRQ_STATUS_CHANGE (0x01 << 3)

#define SERIAL_DIVISOR_LO 0	// If DLAB enabled in LCR

#define SERIAL_DIVISOR_HI 1	// If DLAB enabled in LCR

#define SERIAL_FCR 2
#define SERIAL_FIFO_ENABLE (0x01 << 0)
#define SERIAL_FIFO_CLEAR_RECIEVER (0x01 << 1)
#define SERIAL_FIFO_CLEAR_TRANSMITTER (0x01 << 2)
#define SERIAL_FIFO_DMA_MODE (0x01 << 3)
#define SERIAL_TRIGGER_LEVEL0 (0x01 << 6)
#define SERIAL_TRIGGER_LEVEL1 (0x01 << 7)

#define SERIAL_LCR 3
#define SERIAL_CHARLEN_START (0x01 << 0)
#define SERIAL_STOP_BITS (0x01 << 2)
#define SERIAL_DLAB_ENABLE (0x01 << 3)

#define SERIAL_MCR 4
#define SERIAL_DATA_TERMINAL_READY (0x01 << 0)
#define SERIAL_REQUEST_TO_SEND (0x01 << 1)
#define SERIAL_AUX_OUT1 (0x01 << 2)
#define SERIAL_AUX_OUT2 (0x01 << 3)

#define SERIAL_LSR 5
#define SERIAL_DATA_AVAILABLE (0x01 << 0)
#define SERIAL_TRANSMIT_EMPTY (0x01 << 5)

#define SERIAL_MSR 6

#define SERIAL_SCRATCH 7

static const int serial_ports[4] = { COM1, COM2, COM3, COM4 };

static void serial_init_port(int port)
{
	//Disable iterrupts
	outb(0x00, port + SERIAL_IRQ_ENABLE);
	//Enable DLAB(set baud rate divisor)
	outb(SERIAL_DLAB_ENABLE, port + SERIAL_LCR);
	//Set divisor to 3(lo byte) 38400 baud
	outb(0x03, port + SERIAL_DIVISOR_LO);
	//(hi byte)
	outb(0x00, port + SERIAL_DIVISOR_HI);
	//8 bits, no parity, one stop bit
	outb(SERIAL_CHARLEN_START * 3, port + SERIAL_LCR);
	//Enable FIFO, clear them, with 14 - byte threshold
	outb(SERIAL_FIFO_ENABLE | SERIAL_FIFO_CLEAR_RECIEVER | SERIAL_FIFO_CLEAR_TRANSMITTER | SERIAL_TRIGGER_LEVEL0 | SERIAL_TRIGGER_LEVEL1, port + SERIAL_FCR);
	//IRQs enabled, RTS / DSR set
	outb(SERIAL_DATA_TERMINAL_READY | SERIAL_REQUEST_TO_SEND | SERIAL_AUX_OUT2, port + SERIAL_MCR);
}

static int serial_received(int port)
{
	return inb(port + SERIAL_LSR) & SERIAL_DATA_AVAILABLE;
}

static int is_transmit_empty(int port)
{
	return inb(port + SERIAL_LSR) & SERIAL_TRANSMIT_EMPTY;
}

static int is_valid_port(uint8_t port_no)
{
	return port_no < sizeof(serial_ports) / sizeof(int);
}

char serial_read(uint8_t port_no)
{
	if(!is_valid_port(port_no))
		return -1;

	while(serial_received(serial_ports[port_no]) == 0);
	inb(serial_ports[port_no]);
	return 0;
}

int serial_write(uint8_t port_no, char a)
{
	if(!is_valid_port(port_no))
		return -1;

	while(is_transmit_empty(serial_ports[port_no]) == 0);
	outb(a, serial_ports[port_no]);
	return 0;
}

int serial_device_probe( int unit, int *blocksize, int *nblocks, char *info )
{
	if(unit<0 || unit>3) return 0;
	serial_init_port(serial_ports[unit]);
	*blocksize = 1;
	*nblocks = 0;
	strcpy(info,"serial");
	return 1;
}

int serial_device_read( int unit, void *data, int length, int offset )
{
	int i;
	char *cdata = data;
	for(i=0;i<length;i++) {
		cdata[i] = serial_read(unit);
	}
	return length;
}

int serial_device_write( int unit, const void *data, int length, int offset )
{
	int i;
	const char *cdata = data;
	for(i=0;i<length;i++) {
		serial_write(unit,cdata[i]);
	}
	return length;
}

static struct device_driver serial_driver = {
       .name           = "serial",
       .probe          = serial_device_probe,
       .read           = serial_device_read,
       .read_nonblock  = serial_device_read,
       .write          = serial_device_write
};

void serial_init()
{
	int i;
	for(i = 0; i < sizeof(serial_ports) / sizeof(int); i++) {
		serial_init_port(serial_ports[i]);
	}
	device_driver_register(&serial_driver);
}


