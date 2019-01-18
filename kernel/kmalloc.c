/*
Copyright (C) 2016-2019 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/

#include "kmalloc.h"
#include "console.h"
#include "kernel/types.h"
#include "memorylayout.h"

#define KUNIT sizeof(struct kmalloc_chunk)

#define KMALLOC_STATE_FREE 0xa1a1a1a1
#define KMALLOC_STATE_USED 0xbfbfbfbf

struct kmalloc_chunk {
	int state;
	int length;
	struct kmalloc_chunk *next;
	struct kmalloc_chunk *prev;
};

static struct kmalloc_chunk *head = 0;

/*
Initialize the linked list by creating a single chunk at
a given start address and length.  The chunk is initially
free and has no next or previous chunks.
*/

void kmalloc_init(char *start, int length)
{
	head = (struct kmalloc_chunk *) start;
	head->state = KMALLOC_STATE_FREE;
	head->length = length;
	head->next = 0;
	head->prev = 0;
}

/*
Split a large chunk into two, such that the current chunk
has the desired length, and the next chunk has the remainder.
*/

static void ksplit(struct kmalloc_chunk *c, int length)
{
	struct kmalloc_chunk *n = (struct kmalloc_chunk *) ((char *) c + length);

	n->state = KMALLOC_STATE_FREE;
	n->length = c->length - length;
	n->prev = c;
	n->next = c->next;

	if(c->next)
		c->next->prev = n;

	c->next = n;
	c->length = length;
}

/*
Allocate a chunk of memory of the given length.
To avoid fragmentation, round up the length to
a multiple of the chunk size.  Then, search fo
a chunk of the desired size, and split it if necessary.
*/

void *kmalloc(int length)
{
	// round up length to a multiple of KUNIT
	int extra = length % KUNIT;
	if(extra)
		length += (KUNIT - extra);

	// then add one more unit to accommodate the chunk header
	length += KUNIT;

	struct kmalloc_chunk *c = head;

	while(1) {
		if(!c) {
			printf("kmalloc: out of memory!\n");
			return 0;
		}
		if(c->state == KMALLOC_STATE_FREE && c->length >= length)
			break;
		c = c->next;
	}

	// split the chunk if the remainder is greater than two units
	if(length - c->length > 2 * KUNIT) {
		ksplit(c, length);
	}

	c->state = KMALLOC_STATE_USED;

	// return a pointer to the memory following the chunk header
	return (c + 1);
}

/*
Attempt to merge a chunk with its successor,
if it exists and both are in the free state.
*/

static void kmerge(struct kmalloc_chunk *c)
{
	if(!c)
		return;

	if(c->state != KMALLOC_STATE_FREE)
		return;

	if(c->next && c->next->state == KMALLOC_STATE_FREE) {
		c->length += c->next->length;
		if(c->next->next) {
			c->next->next->prev = c;
		}
		c->next = c->next->next;
	}

}

/*
Free memory by marking the chunk as de-allocated,
then attempting to merge it with the predecessor and successor.
*/

void kfree(void *ptr)
{
	struct kmalloc_chunk *c = (struct kmalloc_chunk *) ptr;
	c--;

	if(c->state != KMALLOC_STATE_USED) {
		printf("invalid kfree(%x)\n", ptr);
		return;
	}

	c->state = KMALLOC_STATE_FREE;

	kmerge(c);
	kmerge(c->prev);
}

void kmalloc_debug()
{
	struct kmalloc_chunk *c;

	printf("state ptr      prev     next     length\n");

	for(c = head; c; c = c->next) {
		if(c->state == KMALLOC_STATE_FREE) {
			printf("F");
		} else if(c->state == KMALLOC_STATE_USED) {
			printf("U");
		} else {
			printf("kmalloc list corrupted at %x!\n", c);
			return;
		}
		printf("     %x %x %x %d\n", c, c->prev, c->next, c->length);
	}
}

// Testing

static void setup(void)
{
	kmalloc_init((char *) KMALLOC_START, KMALLOC_LENGTH);
}

static void tear_down(void)
{
}

static int kmalloc_test_single_alloc(void)
{
	char *ptr = kmalloc(128);
	struct kmalloc_chunk *next = 0;
	int res = (unsigned long) ptr == (unsigned long) head + sizeof(struct kmalloc_chunk);
	res &= head->state == KMALLOC_STATE_USED;
	res &= head->length == 128 + sizeof(struct kmalloc_chunk);
	res &= (char *) head->next == (char *) KMALLOC_START + head->length;
	next = head->next;
	res &= next->state == KMALLOC_STATE_FREE;
	res &= next->length == KMALLOC_LENGTH - head->length;

	return res;
}

static int kmalloc_test_single_alloc_and_free(void)
{
	char *ptr = kmalloc(128);
	int res;
	kfree(ptr);
	res = head->state == KMALLOC_STATE_FREE;
	res &= head->next == 0;
	res &= head->length == KMALLOC_LENGTH;

	return res;
}

int kmalloc_test(void)
{
	int (*tests[]) (void) = {
	kmalloc_test_single_alloc, kmalloc_test_single_alloc_and_free,};

	int i = 0;
	for(i = 0; i < sizeof(tests) / sizeof(tests[0]); i++) {
		printf("running test %d...", i);
		int res;

		setup();
		res = tests[i] ();
		tear_down();

		if(!res) {
			printf("failed\n");
			printf("\ntest %d failed.\n");
			return 0;
		}
		printf("succeeded\n");
	}
	return 1;
}
