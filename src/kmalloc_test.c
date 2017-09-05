#include "kmalloc.h"
#include "memorylayout.h"
#include "console.h"

extern struct kmalloc_chunk *head;

void setup(void)
{
	kmalloc_init((char*)KMALLOC_START, KMALLOC_LENGTH);
}

void tear_down(void)
{
}

int kmalloc_test_single_alloc(void)
{
	char *ptr = kmalloc(128);
	struct kmalloc_chunk *next = 0;
	int res = (unsigned long) ptr == (unsigned long) head + sizeof(struct kmalloc_chunk);
	res &= head->state == KMALLOC_STATE_USED;
	res &= head->length == 128 + sizeof(struct kmalloc_chunk);
	res &= (char *) head->next == (char*) KMALLOC_START + head->length;
	next = head->next;
	res &= next->state == KMALLOC_STATE_FREE;
	res &= next->length == KMALLOC_LENGTH - head->length;
	
	return res;
}

int kmalloc_test_single_alloc_and_free(void)
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
	int (*tests[])(void) = {
		kmalloc_test_single_alloc,
		kmalloc_test_single_alloc_and_free,
	};

	int i = 0;
	for (i = 0; i < sizeof(tests)/sizeof(tests[0]); i++)
	{
		console_printf("running test %d...", i);
		int res;

		setup();
		res = tests[i]();
		tear_down();

		if (!res) {
			console_printf("failed\n");
			console_printf("\ntest %d failed.\n");
			return 0;
		}
		console_printf("succeeded\n");
	}
	return 1;
}
