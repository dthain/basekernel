#ifndef HASH_SET_H
#define HASH_SET_H

#include "kernel/types.h"

struct hash_set *hash_set_create(unsigned max_expected_keys);
void hash_set_delete(struct hash_set *set);

unsigned   hash_set_add(struct hash_set *set, unsigned key, void *data);
void *     hash_set_lookup(struct hash_set *set, unsigned key);
unsigned   hash_set_remove(struct hash_set *set, unsigned key);
unsigned   hash_set_entries(struct hash_set *set);
void       hash_set_print(struct hash_set *set);

#endif
