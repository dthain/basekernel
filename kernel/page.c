/*
Copyright (C) 2015-2019 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/

#include "console.h"
#include "kernel/types.h"
#include "page.h"
#include "string.h"
#include "memorylayout.h"
#include "kernelcore.h"

static uint32_t pages_free = 0;
static uint32_t pages_total = 0;

static uint32_t *freemap = 0;
static uint32_t freemap_bits = 0;
static uint32_t freemap_bytes = 0;
static uint32_t freemap_cells = 0;
static uint32_t freemap_pages = 0;

static void *main_memory_start = (void *) MAIN_MEMORY_START;

#define CELL_BITS (8*sizeof(*freemap))

void page_init()
{
	int i;

	pages_total = (total_memory * 1024 * 1024 - MAIN_MEMORY_START) / PAGE_SIZE;
	pages_free = pages_total;
	printf("memory: %d MB (%d KB) total\n", (pages_free * PAGE_SIZE) / MEGA, (pages_free * PAGE_SIZE) / KILO);

	freemap = main_memory_start;
	freemap_bits = pages_total;
	freemap_bytes = 1 + freemap_bits / 8;
	freemap_cells = 1 + freemap_bits / CELL_BITS;
	freemap_pages = 1 + freemap_bytes / PAGE_SIZE;

	printf("memory: %d bits %d bytes %d cells %d pages\n", freemap_bits, freemap_bytes, freemap_cells, freemap_pages);

	memset(freemap, 0xff, freemap_bytes);
	for(i = 0; i < freemap_pages; i++)
		page_alloc(0);

	// This is ahack that I don't understand yet.
	// vmware doesn't like the use of a particular page
	// close to 1MB, but what it is used for I don't know.

	freemap[0] = 0x0;

	printf("memory: %d MB (%d KB) available\n", (pages_free * PAGE_SIZE) / MEGA, (pages_free * PAGE_SIZE) / KILO);
}

void page_stats( uint32_t *nfree, uint32_t *ntotal )
{
	*nfree = pages_free;
	*ntotal = pages_total;
}

void *page_alloc(bool zeroit)
{
	uint32_t i, j;
	uint32_t cellmask;
	uint32_t pagenumber;
	void *pageaddr;

	if(!freemap) {
		printf("memory: not initialized yet!\n");
		return 0;
	}

	for(i = 0; i < freemap_cells; i++) {
		if(freemap[i] != 0) {
			for(j = 0; j < CELL_BITS; j++) {
				cellmask = (1 << j);
				if(freemap[i] & cellmask) {
					freemap[i] &= ~cellmask;
					pagenumber = i * CELL_BITS + j;
					pageaddr = (pagenumber << PAGE_BITS) + main_memory_start;
					if(zeroit)
						memset(pageaddr, 0, PAGE_SIZE);
					pages_free--;
					return pageaddr;
				}
			}
		}
	}

	printf("memory: WARNING: everything allocated\n");
	halt();

	return 0;
}

void page_free(void *pageaddr)
{
	uint32_t pagenumber = (pageaddr - main_memory_start) >> PAGE_BITS;
	uint32_t cellnumber = pagenumber / CELL_BITS;
	uint32_t celloffset = pagenumber % CELL_BITS;
	uint32_t cellmask = (1 << celloffset);
	freemap[cellnumber] |= cellmask;
	pages_free++;
}
