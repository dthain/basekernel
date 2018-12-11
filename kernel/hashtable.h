#ifndef HASHTABLE_H
#define HASHTABLE_H

#include "kernel/types.h"

struct hash_set *hash_set_create(uint32_t max_expected_keys);
int hash_set_delete(struct hash_set *set);

int  hash_set_add(struct hash_set *set, uint32_t key, void *info);
bool hash_set_lookup(struct hash_set *set, uint32_t key);
bool hash_set_lookup_info(struct hash_set *set, uint32_t key, void **info);
int  hash_set_remove(struct hash_set *set, uint32_t key);
int  hash_set_entries(struct hash_set *set);
void hash_set_print(struct hash_set *set);

#endif
