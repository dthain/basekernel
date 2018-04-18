#ifndef HASHTABLE_H
#define HASHTABLE_H
#define HASHTABLE_PRIME 31
#define HASHTABLE_GOLDEN_RATIO 0x61C88647

#include "kerneltypes.h"

struct hash_set {
	uint32_t total_buckets;
	uint32_t num_entries;
	struct hash_set_node **head;
};

struct hash_set_node {
	uint32_t data;
	void *info;
	uint32_t info_size;
	struct hash_set_node *next;
};

struct hash_set *hash_set_init(uint32_t max_expected_keys);
int hash_set_dealloc(struct hash_set *set);
int hash_set_add(struct hash_set *set, uint32_t key, void *info);
bool hash_set_lookup(struct hash_set *set, uint32_t key);
bool hash_set_lookup_info(struct hash_set *set, uint32_t key, void **info);
int hash_set_delete(struct hash_set *set, uint32_t key);
uint32_t hash_string(char *string, uint32_t range_min, uint32_t range_max);
void debug_print_hash_set(struct hash_set *set);

#endif
