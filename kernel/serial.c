// Derived from: https://wiki.osdev.org/Serial_Ports#Example_Code
// For more info:
//     https://wiki.osdev.org/Serial_Ports
//     http://www.webcitation.org/5ugQv5JOw

#include "kernel/types.h"
#include "kernel/error.h"

#include "interrupt.h"
#include "ioports.h"
#include "process.h"

#define COM1 0x3f8 // irq 4
#define COM2 0x2F8 // irq 3
#define COM3 0x3E8 // irq 4
#define COM4 0x2E8 // irq 3

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

struct list serial_queue = {0,0};

static void serial_interrupt( int intr, int code )
{
	process_wakeup_all(&serial_queue);
}

static void serial_init_port(int port)
{
	interrupt_register(3,serial_interrupt);
	interrupt_register(4,serial_interrupt);

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

static int is_valid_port(int unit)
{
	return unit < sizeof(serial_ports) / sizeof(int);
}

void serial_init()
{
	int i;
	for(i = 0; i < sizeof(serial_ports) / sizeof(int); i++) {
		serial_init_port(serial_ports[i]);
	}
}

int serial_read( int unit, char *data, int length )
{
	if(!is_valid_port(unit))
		return KERROR_INVALID_DEVICE;

	int total = 0;

	while(length>0) {
		while(serial_received(serial_ports[unit]) == 0) {
			process_wait(&serial_queue);
		}
		*data = inb(serial_ports[unit]);
		length--;
		data++;
		total++;
	}

	return total;
}

int serial_write(int unit, const char *data, int length )
{
	if(!is_valid_port(unit))
		return KERROR_INVALID_DEVICE;

	int total = 0;

	while(length>0) {
		while(is_transmit_empty(serial_ports[unit]) == 0) {
			process_wait(&serial_queue);
		}
		outb(*data, serial_ports[unit]);
		length--;
		data++;
		total++;
	}

	return total;
}
