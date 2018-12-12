
#include "bcache.h"
#include "list.h"
#include "memory.h"
#include "kmalloc.h"
#include "string.h"
#include "kernel/error.h"

struct bcache_entry {
	struct list_node node;
	struct device *device;
	int block;
	int dirty;
	char *data;
};

static struct list cache = LIST_INIT;
static int max_cache_size = 100;

struct bcache_entry * bcache_entry_create( struct device *device, int block )
{
	struct bcache_entry *e = kmalloc(sizeof(*e));
	if(!e) return 0;

	e->device = device;
	e->block = block;
	e->data = memory_alloc_page(1);
	if(!e->data) {
		kfree(e);
		return 0;
	}

	return e;

}

void bcache_entry_delete( struct bcache_entry *e )
{
	if(e) {
		if(e->data) memory_free_page(e->data);
		kfree(e);
	}
}

void bcache_entry_clean( struct bcache_entry *e )
{
	if(e->dirty) {
		device_write(e->device,e->data,1,e->block);
		// XXX deal with failure!
		e->dirty = 0;
	}

}

void bcache_init()
{
}

void bcache_trim()
{
	struct bcache_entry *e;

	while(list_size(&cache)>max_cache_size) {
		e = (struct bcache_entry *) list_pop_tail(&cache);
		bcache_entry_clean(e);
		bcache_entry_delete(e);
	}
}

struct bcache_entry * bcache_find( struct device *device, int block )
{
	struct list_node *n;
	struct bcache_entry *e;

	for(n=cache.head;n;n=n->next) {
		e = (struct bcache_entry *)n;
		if(e->device==device && e->block==block) {
			return e;
		}
	}

	return 0;
}

struct bcache_entry * bcache_find_or_create( struct device *device, int block )
{
	struct bcache_entry *e = bcache_find(device,block);
	if(!e) {
		e = bcache_entry_create(device,block);
		if(!e) return 0;
		list_push_head(&cache,&e->node);
	}

	bcache_trim();

	return e;
}

int bcache_read( struct device *device, char *data, int block )
{
	struct bcache_entry *e = bcache_find_or_create(device,block);
	if(!e) return KERROR_NO_MEMORY;

	int result = device_read(device,e->data,1,block);

	if(result>0) {
		memcpy(data,e->data,device_block_size(device));
	} else {
		list_remove(&e->node);
		bcache_entry_delete(e);
	}

	return result;
}

int bcache_write( struct device *device, const char *data, int block )
{
	struct bcache_entry *e = bcache_find_or_create(device,block);
	if(!e) return KERROR_NO_MEMORY;

	memcpy(e->data,data,device_block_size(device));
	e->dirty = 1;

	return 1;
}

void bcache_flush_block( struct device *device, int block )
{
	struct bcache_entry *e;
	e = bcache_find(device,block);
	if(e) bcache_entry_clean(e);
}

void bcache_flush_device( struct device *device )
{
	struct list_node *n;
	struct bcache_entry *e;

	for(n=cache.head;n;n=n->next) {
		e = (struct bcache_entry *) n;
		if(e->device==device) {
			bcache_entry_clean(e);
		}
	}
}
