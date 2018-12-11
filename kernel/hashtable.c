#include "string.h"
#include "hashtable.h"
#include "kmalloc.h"

#define HASHTABLE_PRIME 31
#define HASHTABLE_GOLDEN_RATIO 0x61C88647

struct hash_set {
	int total_buckets;
	int num_entries;
	struct hash_set_node **head;
};

struct hash_set_node {
	int key;
	void *data;
	struct hash_set_node *next;
};

int hash_string(char *string, int range_min, int range_max)
{
	int hash = HASHTABLE_PRIME;
	char *curr = string;
	while(*curr) {
		hash = HASHTABLE_PRIME * hash + *curr;
		curr++;
	}
	return (hash % (range_max - range_min)) + range_min;
}

static int hash_uint(int key, int buckets)
{
	return key * HASHTABLE_GOLDEN_RATIO % buckets;
}

static int hash_set_list_add(struct hash_set_node **head, struct hash_set_node *node)
{
	struct hash_set_node *prev = 0, *curr = *head;
	if(!curr) {
		*head = node;
		return 0;
	}
	while(curr && (curr->key < node->key)) {
		prev = curr;
		curr = curr->next;
	}
	if(!prev && curr->key != node->key) {
		node->next = *head;
		*head = node;
		return 0;
	} else if(!curr) {
		prev->next = node;
		return 0;
	} else if(curr->key != node->key) {
		node->next = curr->next;
		curr->next = node;
		return 0;
	}
	return -1;
}

static struct hash_set_node *hash_set_list_lookup(struct hash_set_node *head, int key)
{
	struct hash_set_node *curr = head;
	while(curr && (curr->key < key)) {
		curr = curr->next;
	}
	return (curr && (curr->key == key)) ? curr : 0;
}

static int hash_set_list_delete(struct hash_set_node **head, int key)
{
	struct hash_set_node *prev = 0, *curr = *head;
	while(curr && (curr->key < key)) {
		prev = curr;
		curr = curr->next;
	}
	if(curr && (curr->key == key)) {
		if(prev)
			prev->next = curr->next;
		if(curr == *head)
			*head = curr->next;
		kfree(curr);
		return 0;
	}
	return -1;
}

struct hash_set *hash_set_create(int buckets)
{
	struct hash_set_node **set_nodes = kmalloc(sizeof(struct hash_set_node *) * buckets);
	struct hash_set *set = kmalloc(sizeof(struct hash_set));
	memset(set_nodes, 0, sizeof(struct hash_set_node *) * buckets);

	set->total_buckets = buckets;
	set->head = set_nodes;
	set->num_entries = 0;

	if(!set || !set_nodes) {
		hash_set_delete(set);
		return 0;
	}
	return set;
}

static int hash_set_list_dealloc(struct hash_set_node *head)
{
	struct hash_set_node *next = head, *curr = head;
	while(curr) {
		next = curr->next;
		kfree(curr);
		curr = next;
	}
	return 0;
}

int hash_set_delete(struct hash_set *set)
{
	struct hash_set_node **set_nodes = set->head;
	if(set)
		kfree(set);
	if(set_nodes) {
		int i;
		for(i = 0; i < set->total_buckets; i++)
			hash_set_list_dealloc(set->head[i]);
		kfree(set_nodes);
	}
	return 0;
}

int hash_set_add(struct hash_set *set, int key, void *data)
{
	int hash_key = hash_uint(key, set->total_buckets);
	struct hash_set_node *node = kmalloc(sizeof(struct hash_set_node));
	node->key = key;
	node->data = data;
	node->next = 0;
	int ret = hash_set_list_add(&(set->head[hash_key]), node);
	if(ret == 0)
		set->num_entries++;
	return ret;
}

void * hash_set_lookup(struct hash_set * set, int key)
{
	int hash_key = hash_uint(key, set->total_buckets);
	struct hash_set_node *result = hash_set_list_lookup(set->head[hash_key], key);
	if(result) {
		return result->data;
	} else {
		return 0;
	}
}

int hash_set_remove(struct hash_set *set, int key)
{
	int hash_key = hash_uint(key, set->total_buckets);
	int result = hash_set_list_delete(&(set->head[hash_key]), key);
	if(result == 0)
		set->num_entries--;
	return result;
}

int hash_set_entries( struct hash_set *set )
{
	return set->num_entries;
}

void hash_set_print(struct hash_set *set)
{
	int i;
	printf("printing hash set:\n");
	for(i = 0; i < set->total_buckets; i++) {
		struct hash_set_node *start = set->head[i];
		while(start) {
			printf("%u: %u\n", i, start->key);
			start = start->next;
		}
	}
}
