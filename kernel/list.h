/*
Copyright (C) 2015-2019 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/

#ifndef LIST_H
#define LIST_H

struct list;
struct list_node;

struct list {
	struct list_node *head;
	struct list_node *tail;
	int size;
};

struct list_node {
	struct list_node *next;
	struct list_node *prev;
	struct list *list;
	int priority;
};

#define LIST_INIT {0,0}

void list_push_head(struct list *list, struct list_node *node);
void list_push_tail(struct list *list, struct list_node *node);
void list_push_priority(struct list *list, struct list_node *node, int pri);
struct list_node *list_pop_head(struct list *list);
struct list_node *list_pop_tail(struct list *list);
void list_remove(struct list_node *n);
int  list_size(struct list *list);

#endif
