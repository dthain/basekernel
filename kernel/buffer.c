#include "hashtable.h"
#include "list.h"
#include "kmalloc.h"
#include "ata.h"
#include "string.h"
#include "pagetable.h"
#include "memory.h"
#include "device.h"

#define CACHE_SIZE 51

struct buffer_entry {
	struct list_node node;
	uint32_t block_no;
	unsigned char *data;
};

struct buffer {
	struct list cache;
	struct hash_set *cache_map;
	int block_size;
};

struct buffer *buffer_init(int block_size)
{
	struct buffer *ret = kmalloc(sizeof(struct buffer));
	ret->block_size = block_size;
	ret->cache_map = hash_set_create(CACHE_SIZE + 1);
	return ret;
}

int buffer_read(struct buffer *buf, int block, void *data)
{
	struct buffer_entry *cache_entry =
		hash_set_lookup(buf->cache_map, block);

	if(cache_entry) {
		memcpy(data, cache_entry->data, buf->block_size);
		return 0;
	} else {
		return -1;
	}
}

int buffer_delete(struct buffer *buf, int block)
{
	struct buffer_entry *data =
		hash_set_lookup(buf->cache_map, block);

	if(!data) return -1;

	memory_free_page(data->data);
	list_remove((struct list_node *) data);
	if(hash_set_remove(buf->cache_map, block) < 0) {
		return -1;
	}
	return 0;
}

int buffer_drop_lru(struct buffer *buf)
{
	struct buffer_entry *current_cache_data = (struct buffer_entry *) list_pop_tail(&buf->cache);
	hash_set_remove(buf->cache_map, current_cache_data->block_no);
	return 0;
}

int buffer_add(struct buffer *buf, int block, const void *data)
{
	struct buffer_entry *write = 0;
	if(hash_set_entries(buf->cache_map) >= CACHE_SIZE)
		buffer_drop_lru(buf);
	if(hash_set_lookup(buf->cache_map, block)) {
		buffer_delete(buf, block);
	}
	write = kmalloc(sizeof(struct buffer_entry));
	write->block_no = block;
	write->data = memory_alloc_page(1);
	memcpy(write->data, data, buf->block_size);
	if(hash_set_add(buf->cache_map, block, write) < 0) {
		kfree(write);
		return -1;
	}
	list_push_head(&buf->cache, (struct list_node *) write);
	return 0;
}
