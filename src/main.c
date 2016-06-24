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
#include "cdromfs.h"
#include "string.h"
#include "graphics.h"
#include "ascii.h"
#include "syscall.h"
#include "rtc.h"
#include "kernelcore.h"
#include "kmalloc.h"
#include "memorylayout.h"


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

	console_printf("video: %d x %d\n",video_xres,video_yres,video_xbytes);
	console_printf("kernel: %d bytes\n",kernel_size);

	memory_init();
	kmalloc_init((char*)KMALLOC_START,KMALLOC_LENGTH);
	interrupt_init();
	rtc_init();
	clock_init();
	mouse_init();
	keyboard_init();
	process_init();
	ata_init();

	struct cdrom_volume *v = cdrom_volume_open(1);
	if(v) {
		struct cdrom_dirent *d = cdrom_volume_root(v);
		if(d) {
			cdrom_dirent_readdir(d,0,1);
			cdrom_dirent_close(d);
		} else {
			printf("couldn't access root dir!\n");
		}
		cdrom_volume_close(v);
	} else {
		printf("couldn't mount filesystem!\n");
	}

	console_printf("\nBASEKERNEL READY:\n");

	while(1) console_putchar(keyboard_read());

	return 0;
}
