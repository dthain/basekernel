#include "hashtable.h"
#include "list.h"
#include "kmalloc.h"
#include "ata.h"
#include "string.h"

#define ATA_CACHE_SIZE 50

static struct list caches[4];
static struct hash_set *cache_map[4];

struct ata_cache_entry {
	struct list_node node;
	uint32_t block_no;
	unsigned char data[ATA_BLOCKSIZE];
};

static int ata_cache_read(int id, int block, struct ata_cache_entry *data) {
	void *read;
	struct ata_cache_entry *cache_entry = 0;
	int exists = hash_set_lookup_info(cache_map[id], block, &read);
	if (exists) {
		cache_entry = read;
		memcpy(data, cache_entry, sizeof(struct ata_cache_entry));
		return 0;
	}
	return -1;
}

static int ata_cache_delete (int id, int block){
	void **data = 0;
	struct ata_cache_entry *current_cache_data = 0;
	if (!hash_set_lookup_info(cache_map[id], block, data)) {
		return -1;
	}
	list_remove((struct list_node*) current_cache_data);
	if (hash_set_delete(cache_map[id], block) < 0) {
		return -1;
	}
	return 0;
}

static int ata_cache_drop_lru(int id) {
	struct ata_cache_entry *current_cache_data = (struct ata_cache_entry *) list_pop_tail(&caches[id]);
	hash_set_delete(cache_map[id], current_cache_data->block_no);
	return 0;
}

static int ata_cache_add(int id, int block, struct ata_cache_entry *data) {
	struct ata_cache_entry *write = 0;
	if (cache_map[id]->num_entries == ATA_CACHE_SIZE) ata_cache_drop_lru(id);
	int exists = hash_set_lookup(cache_map[id], block);
	if (exists) ata_cache_delete(id, block);
	write = kmalloc(sizeof(struct ata_cache_entry));
	memcpy(write, data, sizeof(struct ata_cache_entry));
	if (hash_set_add(cache_map[id], block, write) < 0) {
		kfree(write);
		return -1;
	}
	list_push_head(&caches[id], (struct list_node *) write);
	return 0;
}

int ata_buffer_read(int id, int block, void *buffer) {
	struct ata_cache_entry cache_entry;
	if (ata_cache_read(id, block, &cache_entry) == 0) {
		memcpy(buffer, cache_entry.data, ATA_BLOCKSIZE);
	}
	else { 
		ata_read(id, buffer, 1, block);
		memcpy(cache_entry.data, buffer, ATA_BLOCKSIZE);
		cache_entry.block_no = block;
		ata_cache_add(id, block, &cache_entry);
	}
	return 0;
}

int ata_buffer_write(int id, int block, void *buffer) {
	ata_cache_delete(id, block);
	ata_write(id, buffer, 1, block);
	return 0;
}
