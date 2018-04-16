/*
Copyright (C) 2015 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/

#include "console.h"
#include "memory.h"
#include "process.h"
#include "interrupt.h"
#include "keyboard.h"
#include "mouse.h"
#include "clock.h"
#include "ata.h"
#include "device.h"
#include "cdromfs.h"
#include "string.h"
#include "graphics.h"
#include "ascii.h"
#include "syscall.h"
#include "rtc.h"
#include "kernelcore.h"
#include "kmalloc.h"
#include "memorylayout.h"
#include "kshell.h"
#include "cdromfs.h"
#include "kevinfs/kevinfs_test.h"
#include "kevinfs/kevinfs.h"
#include "serial.h"

struct dirent *root_directory = 0;
struct dirent *current_directory = 0;

/*
This is the C initialization point of the kernel.
By the time we reach this point, we are in protected mode,
with interrupts disabled, a valid C stack, but no malloc heap.
Now we initialize each subsystem in the proper order:
*/

int kernel_main()
{
	struct graphics *g = graphics_create_root();

	console_init(g);
#ifdef TEST
	serial_init();
#endif

	console_printf("video: %d x %d\n",video_xres,video_yres,video_xbytes);
	console_printf("kernel: %d bytes\n",kernel_size);

	memory_init();
	kmalloc_init((char*)KMALLOC_START,KMALLOC_LENGTH);
	interrupt_init();
	device_init();
	rtc_init();
	clock_init();
	mouse_init();
	keyboard_init();
	process_init();
	ata_init();
	cdrom_init();
	kevinfs_init();

	console_printf("\nKERNEL SHELL READY:\n");
	kshell_launch();

	while(1) console_putchar(keyboard_read());

	return 0;
}
