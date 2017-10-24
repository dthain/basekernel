#ifndef FS_TRANSACTION_H
#define FS_TRANSACTION_H
#include "kevinfs.h"

enum kevinfs_transaction_data_type
{
	FS_TRANSACTION_BLOCK,
	FS_TRANSACTION_INODE,
};

enum kevinfs_transaction_op_type
{
	FS_TRANSACTION_CREATE,
	FS_TRANSACTION_MODIFY,
	FS_TRANSACTION_DELETE,
};

struct kevinfs_transaction_entry
{
	enum kevinfs_transaction_op_type op;
	enum kevinfs_transaction_data_type data_type;
	bool is_completed;
	uint32_t number;
	union {
		struct kevinfs_inode node;
		uint8_t to_write[FS_BLOCKSIZE];
		uint8_t to_revert[FS_BLOCKSIZE];
	} data;
	struct kevinfs_transaction_entry *next;
	struct kevinfs_transaction_entry *prev;
};

struct kevinfs_transaction
{
	struct kevinfs_transaction_entry *head;
};

int kevinfs_transactions_init(struct kevinfs_superblock *s_original);
void kevinfs_transaction_init(struct kevinfs_transaction *t);
int kevinfs_transaction_stage_inode(struct kevinfs_transaction *t, struct kevinfs_inode *inode, enum kevinfs_transaction_op_type op);
int kevinfs_transaction_stage_data(struct kevinfs_transaction *t, uint32_t index, uint8_t *buffer, enum kevinfs_transaction_op_type op);
int kevinfs_transaction_commit(struct kevinfs_transaction *t);

#endif
