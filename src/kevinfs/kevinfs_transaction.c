#include "../string.h"
#include "kevinfs_ata.h"
#include "kevinfs.h"
#include "../kmalloc.h"

#include "kevinfs_transaction.h"

static struct kevinfs_superblock *super;

static void kevinfs_transaction_append(struct kevinfs_transaction *t, struct kevinfs_transaction_entry *entry)
{
	struct kevinfs_transaction_entry *current = t->head, *prev = 0;
	while (current && !(entry->data_type == current->data_type && entry->number == current->number)) {
		prev = current;
		current = current->next;
	}
	if (prev) {
		prev->next = entry;
		entry->prev = prev;
	}
	else {
		t->head = entry;
	}
	if (current) {
		if (entry->op == FS_TRANSACTION_MODIFY && current->op == FS_TRANSACTION_CREATE)
			entry->op = FS_TRANSACTION_CREATE;
		entry->next = current->next;
		kfree(current);
		entry->next->prev = entry;
	}
}

static int kevinfs_do_delete_inode(struct kevinfs_transaction_entry *entry)
{
	uint32_t inode_number = entry->number;
	uint32_t index = inode_number - 1;

	if (kevinfs_ata_unset_bit(index, super->inode_bitmap_start, super->inode_start) < 0)
		return -1;

	entry->op = FS_TRANSACTION_CREATE;
	entry->is_completed = 1;
	memset(&entry->data.node, 0, sizeof(struct kevinfs_inode));

	return 0;
}

static int kevinfs_do_save_inode(struct kevinfs_transaction_entry *entry)
{
	uint32_t index = entry->number - 1;
	uint32_t inodes_per_block = FS_BLOCKSIZE / sizeof(struct kevinfs_inode);
	uint32_t block = index / inodes_per_block;
	uint32_t offset = index % inodes_per_block;
	struct kevinfs_inode temp;
	struct kevinfs_inode current_nodes[inodes_per_block];

	if (entry->op == FS_TRANSACTION_CREATE) {
		if (kevinfs_ata_set_bit(index, super->inode_bitmap_start, super->inode_start) < 0) {
			return -1;
		}
		entry->op = FS_TRANSACTION_DELETE;
		entry->is_completed = 1;
	}

	if (entry->data.node.inode_number) {
		if (kevinfs_ata_read_block(super->inode_start + block, current_nodes) < 0)
			return -1;
		memcpy(&temp, current_nodes + offset, sizeof(struct kevinfs_inode));
		memcpy(current_nodes + offset, &entry->data.node, sizeof(struct kevinfs_inode));
		if (kevinfs_ata_write_block(super->inode_start + block, current_nodes) < 0)
			return -1;
		entry->data.node = temp;
		entry->is_completed = 1;
	}

	return 0;
}

static int kevinfs_do_delete_data(struct kevinfs_transaction_entry *entry)
{
	uint32_t index = entry->number;

	if (kevinfs_ata_unset_bit(index, super->block_bitmap_start, super->free_block_start) < 0) {
		return -1;
	}

	entry->op = FS_TRANSACTION_CREATE;
	entry->is_completed = 1;
	return 0;
}

static int kevinfs_do_save_data(struct kevinfs_transaction_entry *entry)
{
	uint32_t index = entry->number;
	uint8_t temp[FS_BLOCKSIZE];
	bool is_valid;


	if (entry->op == FS_TRANSACTION_CREATE) {
		if (kevinfs_ata_set_bit(index, super->block_bitmap_start, super->free_block_start) < 0) {
			return -1;
		}
		entry->op = FS_TRANSACTION_DELETE;
		entry->is_completed = 1;
	}
	else if (kevinfs_ata_check_bit(index, super->block_bitmap_start, super->free_block_start, &is_valid) < 0 || !is_valid)
		return -1;


	memcpy(temp, entry->data.to_write, sizeof(temp));
	if (kevinfs_ata_write_block(super->free_block_start + index, temp) < 0)
		return -1;
	entry->is_completed = 1;

	return 0;
}

int kevinfs_transactions_init(struct kevinfs_superblock *s_original)
{
	super = s_original;
	return 0;
}

void kevinfs_transaction_init(struct kevinfs_transaction *t)
{
	struct kevinfs_transaction_entry *current = t->head, *next = t->head;
	while (current) {
		next = next->next;
		kfree(current);
		current = next;
	}
	t->head = 0;
}

int kevinfs_transaction_stage_inode(struct kevinfs_transaction *t, struct kevinfs_inode *node, enum kevinfs_transaction_op_type op)
{
	struct kevinfs_transaction_entry *entry = kmalloc(sizeof(struct kevinfs_transaction_entry));

	if (!entry)
		return -1;

	memset(entry, 0, sizeof(struct kevinfs_transaction_entry));
	entry->data_type = FS_TRANSACTION_INODE;
	entry->number = node->inode_number;
	entry->op = op;
	entry->data.node = *node;

	kevinfs_transaction_append(t, entry);

	return 0;
}

int kevinfs_transaction_stage_data(struct kevinfs_transaction *t, uint32_t index, uint8_t *buffer, enum kevinfs_transaction_op_type op)
{
	struct kevinfs_transaction_entry *entry = kmalloc(sizeof(struct kevinfs_transaction_entry));

	if (!entry)
		return -1;

	memset(entry, 0, sizeof(struct kevinfs_transaction_entry));
	entry->data_type = FS_TRANSACTION_BLOCK;
	entry->number = index;
	entry->op = op;

	if (op == FS_TRANSACTION_MODIFY)
		memcpy(entry->data.to_write, buffer, FS_BLOCKSIZE);

	kevinfs_transaction_append(t, entry);

	return 0;
}

static int kevinfs_try_commit_entry(struct kevinfs_transaction_entry *entry)
{
	int ret = -1;
	if (entry->data_type == FS_TRANSACTION_INODE) {
		switch (entry->op) {
			case FS_TRANSACTION_CREATE:
			case FS_TRANSACTION_MODIFY:
				ret = kevinfs_do_save_inode(entry);
				break;
			case FS_TRANSACTION_DELETE:
				ret = kevinfs_do_delete_inode(entry);
				break;
		}
	}
	else if (entry->data_type == FS_TRANSACTION_BLOCK) {
		switch (entry->op) {
			case FS_TRANSACTION_CREATE:
			case FS_TRANSACTION_MODIFY:
				ret = kevinfs_do_save_data(entry);
				break;
			case FS_TRANSACTION_DELETE:
				ret = kevinfs_do_delete_data(entry);
				break;
		}
	}
	return ret;
}

static int kevinfs_try_commit(struct kevinfs_transaction *t, struct kevinfs_transaction_entry **last_successful)
{
	struct kevinfs_transaction_entry *position = t->head;
	while (position) {
		int ret = kevinfs_try_commit_entry(position);
		if (ret < 0) {
			printf("fs: commit failed\n");
			*last_successful = position;
			return ret;
		}
		position = position->next;
	}
	return 0;
}

static int kevinfs_rollback(struct kevinfs_transaction *t, struct kevinfs_transaction_entry *last_successful)
{
	struct kevinfs_transaction_entry *position = last_successful;
	while (position) {
		int ret;
		if (!position->is_completed) {
			position = position->prev;
			continue;
		}
		ret = kevinfs_try_commit_entry(position);
		if (ret < 0) {
			printf("fs: rollback failed\n");
			return ret;
		}
		position = position->prev;
	}
	return 0;
}

int kevinfs_transaction_commit(struct kevinfs_transaction *t)
{
	struct kevinfs_transaction_entry *last_successful;
	int ret = kevinfs_try_commit(t, &last_successful);
	if (ret < 0)
		kevinfs_rollback(t, last_successful);
	return ret;
}
