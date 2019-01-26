/*
Copyright (C) 2015-2019 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/

#include "bcache.h"
#include "list.h"
#include "page.h"
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
static struct bcache_stats stats = {0};
static int max_cache_size = 100;

struct bcache_entry * bcache_entry_create( struct device *device, int block )
{
	struct bcache_entry *e = kmalloc(sizeof(*e));
	if(!e) return 0;

	e->device = device;
	e->block = block;
	e->data = page_alloc(1);
	if(!e->data) {
		kfree(e);
		return 0;
	}

	return e;

}

void bcache_entry_delete( struct bcache_entry *e )
{
	if(e) {
		if(e->data) page_free(e->data);
		kfree(e);
	}
}

void bcache_entry_clean( struct bcache_entry *e )
{
	if(e->dirty) {
		device_write(e->device,e->data,1,e->block);
		// XXX How to deal with failure here?
		e->dirty = 0;
		stats.writebacks++;
	}

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

struct bcache_entry * bcache_find_or_create( struct device *device, int block, int *was_a_hit )
{
	struct bcache_entry *e = bcache_find(device,block);
	if(e) {
		*was_a_hit = 1;
	} else {
		*was_a_hit = 0;
		e = bcache_entry_create(device,block);
		if(!e) return 0;
		list_push_head(&cache,&e->node);
	}

	bcache_trim();

	return e;
}

int bcache_read_block( struct device *device, char *data, int block )
{
	int hit=0;
	int result;

	struct bcache_entry *e = bcache_find_or_create(device,block,&hit);
	if(!e) return KERROR_OUT_OF_MEMORY;

	if(hit) {
		stats.read_hits++;
		result = 1;
	} else {
		stats.read_misses++;
		result = device_read(device,e->data,1,block);
	}

	if(result>0) {
		memcpy(data,e->data,device_block_size(device));
	} else {
		list_remove(&e->node);
		bcache_entry_delete(e);
	}

	return result;
}

int bcache_read( struct device *device, char *data, int blocks, int offset )
{
	int i,r;
	int count = 0;
	int bs = device_block_size(device);

	for(i=0;i<blocks;i++) {
		r = bcache_read_block(device,&data[i*bs],offset+i);
		if(r<1) break;
		count++;
	}

	if(count>0) {
		return count;
	} else {
		return r;
	}
}


int bcache_write_block( struct device *device, const char *data, int block )
{
	int hit;

	struct bcache_entry *e = bcache_find_or_create(device,block,&hit);
	if(!e) return KERROR_OUT_OF_MEMORY;

	if(hit) {
		stats.write_hits++;
	} else {
		stats.write_misses++;
	}

	memcpy(e->data,data,device_block_size(device));
	e->dirty = 1;

	return 1;
}

int bcache_write( struct device *device, const char *data, int blocks, int offset )
{
	int i,r;
	int count = 0;
	int bs = device_block_size(device);

	for(i=0;i<blocks;i++) {
		r = bcache_write_block(device,&data[i*bs],offset+i);
		if(r<1) break;
		count++;
	}

	if(count>0) {
		return count;
	} else {
		return r;
	}
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

void bcache_flush_all()
{
	struct list_node *n;
	struct bcache_entry *e;

	for(n=cache.head;n;n=n->next) {
		e = (struct bcache_entry *) n;
		bcache_entry_clean(e);
	}
}

void bcache_get_stats( struct bcache_stats *s )
{
	memcpy(s,&stats,sizeof(*s));
}
