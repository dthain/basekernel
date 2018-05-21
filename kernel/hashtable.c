#include "string.h"
#include "hashtable.h"
#include "kmalloc.h"

uint32_t hash_string(char *string, uint32_t range_min, uint32_t range_max)
{
	uint32_t hash = HASHTABLE_PRIME;
	char *curr = string;
	while (*curr) {
		hash = HASHTABLE_PRIME * hash + *curr;
		curr++;
	}
	return (hash % (range_max - range_min)) + range_min;
}

static uint32_t hash_uint(uint32_t key, uint32_t buckets)
{
	return key * HASHTABLE_GOLDEN_RATIO % buckets;
}

static int hash_set_list_add(struct hash_set_node **head, struct hash_set_node *node)
{
	struct hash_set_node *prev = 0, *curr = *head;
	if (!curr) {
		*head = node;
		return 0;
	}
	while (curr && (curr->data < node->data)) {
		prev = curr;
		curr = curr->next;
	}
	if (!prev && curr->data != node->data) {
		node->next = *head;
		*head = node;
		return 0;
	}
	else if (!curr) {
		prev->next = node;
		return 0;
	}
	else if (curr->data != node->data) {
		node->next = curr->next;
		curr->next = node;
		return 0;
	}
	return -1;
}

static struct hash_set_node *hash_set_list_lookup(struct hash_set_node *head, uint32_t key)
{
	struct hash_set_node *curr = head;
	while (curr && (curr->data < key)) {
		curr = curr->next;
	}
	return (curr && (curr->data == key)) ? curr : 0;
}

static int hash_set_list_delete(struct hash_set_node **head, uint32_t key)
{
	struct hash_set_node *prev = 0, *curr = *head;
	while (curr && (curr->data < key)) {
		prev = curr;
		curr = curr->next;
	}
	if (curr && (curr->data == key)) {
		if (prev)
			prev->next = curr->next;
		if (curr == *head)
			*head = curr->next;
		kfree(curr);
		return 0;
	}
	return -1;
}

bool hash_set_lookup_info(struct hash_set *set, uint32_t key, void **data)
{
	uint32_t hash_key = hash_uint(key, set->total_buckets);
	struct hash_set_node *result = hash_set_list_lookup(set->head[hash_key], key);
	if (result != 0)
		*data = result->info;
	return result != 0;
}

struct hash_set *hash_set_init(uint32_t buckets)
{
	struct hash_set_node **set_nodes = kmalloc(sizeof(struct hash_set_node *) * buckets);
	struct hash_set *set = kmalloc(sizeof(struct hash_set));
	memset(set_nodes, 0, sizeof(struct hash_set_node *) * buckets);

	set->total_buckets = buckets;
	set->head = set_nodes;
	set->num_entries = 0;

	if (!set || !set_nodes) {
		hash_set_dealloc(set);
		return 0;
	}
	return set;
}

static int hash_set_list_dealloc(struct hash_set_node *head)
{
	struct hash_set_node *next = head, *curr = head;
	while (curr) {
		next = curr->next;
		kfree(curr);
		curr = next;
	}
	return 0;
}

int hash_set_dealloc(struct hash_set *set)
{
	struct hash_set_node **set_nodes = set->head;
	if (set)
		kfree(set);
	if (set_nodes) {
		uint32_t i;
		for (i = 0; i < set->total_buckets; i++)
			hash_set_list_dealloc(set->head[i]);
		kfree(set_nodes);
	}
	return 0;
}

int hash_set_add(struct hash_set *set, uint32_t key, void *info)
{
	uint32_t hash_key = hash_uint(key, set->total_buckets);
	struct hash_set_node *node = kmalloc(sizeof(struct hash_set_node));
	node->data = key;
	node->info = info;
	node->next = 0;
	int ret = hash_set_list_add(&(set->head[hash_key]), node);
        if (ret == 0) set->num_entries++;
	return ret;
}

bool hash_set_lookup(struct hash_set *set, uint32_t key)
{
	uint32_t hash_key = hash_uint(key, set->total_buckets);
	struct hash_set_node *result = hash_set_list_lookup(set->head[hash_key], key);
	return result != 0;
}

int hash_set_delete(struct hash_set *set, uint32_t key)
{
	uint32_t hash_key = hash_uint(key, set->total_buckets);
	int result = hash_set_list_delete(&(set->head[hash_key]), key);
        if (result == 0) set->num_entries--;
	return result;
}

void debug_print_hash_set(struct hash_set *set)
{
	uint32_t i;
	printf("printing hash set:\n");
	for (i = 0; i < set->total_buckets; i++) {
		struct hash_set_node *start = set->head[i];
		while(start) {
			printf("%u: %u\n", i, start->data);
			start = start->next;
		}
	}
}
