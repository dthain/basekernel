/*
Copyright (C) 2015-2019 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/

#include "kernel/types.h"
#include "kernel/error.h"
#include "kmalloc.h"
#include "diskfs.h"
#include "string.h"
#include "fs.h"
#include "fs_internal.h"
#include "bcache.h"
#include "page.h"

/* Read or write a block from the raw device, starting from zero. */

static int diskfs_block_read(struct device *d, struct diskfs_block *b, uint32_t blockno )
{
	return bcache_read(d, b->data, 1, blockno) ? DISKFS_BLOCK_SIZE : -1;
}

static int diskfs_block_write(struct device *d, struct diskfs_block *b, uint32_t blockno )
{
	return bcache_write(d, b->data, 1, blockno) ? DISKFS_BLOCK_SIZE : -1;
}

/* Read or write a bitmap block, starting from the bitmap offset. */

static int diskfs_bitmap_block_read(struct fs_volume *v, struct diskfs_block *b, uint32_t blockno )
{
	if(blockno>=v->disk.bitmap_blocks) return KERROR_OUT_OF_SPACE;
	return diskfs_block_read(v->device,b,v->disk.bitmap_start+blockno);
}

static int diskfs_bitmap_block_write(struct fs_volume *v, struct diskfs_block *b, uint32_t blockno )
{
	if(blockno>=v->disk.bitmap_blocks) return KERROR_OUT_OF_SPACE;
	return diskfs_block_write(v->device,b,v->disk.bitmap_start+blockno);
}

/* Read or write an inode block, starting from the inode block offset. */

static int diskfs_inode_block_read(struct fs_volume *v, struct diskfs_block *b, uint32_t blockno )
{
	if(blockno>=v->disk.inode_blocks) return KERROR_OUT_OF_SPACE;
	return diskfs_block_read(v->device,b,v->disk.inode_start+blockno);
}

static int diskfs_inode_block_write(struct fs_volume *v, struct diskfs_block *b, uint32_t blockno )
{
	if(blockno>=v->disk.inode_blocks) return KERROR_OUT_OF_SPACE;
	return diskfs_block_write(v->device,b,v->disk.inode_start+blockno);
}

/* Read or write a data block, starting from the data block offset. */

static int diskfs_data_block_read(struct fs_volume *v, struct diskfs_block *b, uint32_t blockno )
{
	if(blockno>=v->disk.data_blocks) return KERROR_OUT_OF_SPACE;
	return diskfs_block_read(v->device,b,v->disk.data_start+blockno);
}

static int diskfs_data_block_write(struct fs_volume *v, struct diskfs_block *b, uint32_t blockno )
{
	if(blockno>=v->disk.data_blocks) return KERROR_OUT_OF_SPACE;
	return diskfs_block_write(v->device,b,v->disk.data_start+blockno);
}

/*
Allocate a new data block by scanning the bitmap.
If available, return the block number.
If nothing available, return zero.
*/

static uint32_t diskfs_data_block_alloc( struct fs_volume *v )
{
	struct diskfs_block *b = page_alloc(0);
	struct diskfs_superblock *s= &v->disk;
	int i, j, k;

	for(i=0;i<s->bitmap_blocks;i++) {
		diskfs_bitmap_block_read(v,b,i);
		for(j=0;j<DISKFS_BLOCK_SIZE;j++) {
			if(b->data[j]!=0xff) {
				for(k=0;k<8;k++) {
					if(!((1<<k) & b->data[j])) {
						int blockno = i*DISKFS_BLOCK_SIZE+j*8+k;

						// Never allocate block zero;
						if(blockno==0) continue;

						// Do not exceet the actual number of blocks
						if(blockno>=v->disk.data_blocks) break;

						b->data[j] |= 1<<k;
						diskfs_bitmap_block_write(v,b,i);
						page_free(b);
						return blockno;
					}
				}
			}		
		}
	}

	printf("diskfs: warning: out of space!\n");

	page_free(b);
	return 0;
}

static void diskfs_data_block_free( struct fs_volume *v, int blockno )
{
	struct diskfs_block *b = page_alloc(0);

	int bitmap_block = blockno/DISKFS_BLOCK_SIZE;
	int bitmap_byte = blockno%DISKFS_BLOCK_SIZE/8;
	int bitmap_bit = blockno%DISKFS_BLOCK_SIZE%8;

	diskfs_bitmap_block_read(v,b,bitmap_block);
	b->data[bitmap_byte] &= ~(1<<bitmap_bit);
	diskfs_bitmap_block_write(v,b,bitmap_block);

	page_free(b);
}

static int diskfs_inumber_alloc( struct fs_volume *v )
{
	struct diskfs_block *b = page_alloc(0);
	int i, j;

	for(i=0;i<v->disk.inode_blocks;i++) {
		diskfs_inode_block_read(v,b,i);
		for(j=0;j<DISKFS_INODES_PER_BLOCK;j++) {
			if(!b->inodes[j].inuse) {
				int inumber = i * DISKFS_INODES_PER_BLOCK + j;
				b->inodes[j].inuse = 1;
				diskfs_inode_block_write(v,b,i);
				page_free(b);
				return inumber;
			}
		}
	}

	printf("diskfs: warning: out of inodes!\n");

	page_free(b);
	return 0;
}

static void diskfs_inumber_free( struct fs_volume *v, int inumber )
{
	int inode_block = inumber / DISKFS_INODES_PER_BLOCK;
	struct diskfs_block *b = page_alloc(0);
	diskfs_inode_block_read(v,b,inode_block);
	b->inodes[inumber%DISKFS_INODES_PER_BLOCK].inuse = 0;
	diskfs_inode_block_write(v,b,inode_block);
	page_free(b);
}

int diskfs_inode_load( struct fs_volume *v, int inumber, struct diskfs_inode *inode )
{
	struct diskfs_block *b = page_alloc(0);

	int inode_block = inumber / DISKFS_INODES_PER_BLOCK;
	int inode_position = inumber % DISKFS_INODES_PER_BLOCK;

	diskfs_inode_block_read(v,b,inode_block);
	memcpy(inode,&b->inodes[inode_position],sizeof(*inode));

	page_free(b);

	return 1;
}

int diskfs_inode_save( struct fs_volume *v, int inumber, struct diskfs_inode *inode )
{
	struct diskfs_block *b = page_alloc(0);

	int inode_block = inumber / DISKFS_INODES_PER_BLOCK;
	int inode_position = inumber % DISKFS_INODES_PER_BLOCK;

	diskfs_inode_block_read(v,b,inode_block);
	memcpy(&b->inodes[inode_position],inode,sizeof(*inode));
	diskfs_inode_block_write(v,b,inode_block);

	page_free(b);

	return 1;
}

int diskfs_inode_read( struct fs_dirent *d, struct diskfs_block *b, uint32_t block )
{
	int actual;

	if(block<DISKFS_DIRECT_POINTERS) {
		actual = d->disk.direct[block];
	} else {
		diskfs_data_block_read(d->volume,b,d->disk.indirect);
		actual = b->pointers[block-DISKFS_DIRECT_POINTERS];
	}
		
	return diskfs_data_block_read(d->volume,b,actual);
}

int diskfs_inode_write( struct fs_dirent *d, struct diskfs_block *b, uint32_t block )
{
	int actual;

	struct diskfs_inode *i = &d->disk;

	if(block<DISKFS_DIRECT_POINTERS) {
		actual = i->direct[block];
		if(actual==0) {
			actual = diskfs_data_block_alloc(d->volume);
			if(actual==0) return KERROR_OUT_OF_SPACE;
			i->direct[block] = actual;
			diskfs_inode_save(d->volume,d->inumber,i);	
		}
	} else {
		struct diskfs_block *iblock = page_alloc(0);

		if(i->indirect==0) {
			actual = diskfs_data_block_alloc(d->volume);
			if(actual==0) {
				page_free(iblock);
				return KERROR_OUT_OF_SPACE;
			}
			i->indirect = actual;
			diskfs_inode_save(d->volume,d->inumber,i);	
			memset(iblock,0,DISKFS_BLOCK_SIZE);
			diskfs_data_block_write(d->volume,iblock,i->indirect);
		}

		diskfs_data_block_read(d->volume,iblock,i->indirect);
		actual = iblock->pointers[block-DISKFS_DIRECT_POINTERS];
		if(actual==0) {
			actual = diskfs_data_block_alloc(d->volume);
			if(actual==0) {
				page_free(iblock);
				return KERROR_OUT_OF_SPACE;
			}
			iblock->pointers[block-DISKFS_DIRECT_POINTERS] = actual;
			diskfs_data_block_write(d->volume,iblock,i->indirect);
		}
		page_free(iblock);
	}

	return diskfs_data_block_write(d->volume,b,actual);
}

struct fs_dirent * diskfs_dirent_create( struct fs_volume *volume, int inumber, int type )
{
	struct fs_dirent *d = kmalloc(sizeof(*d));
	memset(d,0,sizeof(*d));

	diskfs_inode_load(volume,inumber,&d->disk);

	d->volume = volume;
	d->size = d->disk.size;
	d->inumber = inumber;
	d->refcount = 1;
	d->isdir = type==DISKFS_ITEM_DIR;
	return d;
}


int diskfs_dirent_close( struct fs_dirent *d )
{
	// XXX check if inode dirty first
	diskfs_inode_save(d->volume,d->inumber,&d->disk);
	return 0;
}

struct fs_dirent * diskfs_dirent_lookup( struct fs_dirent *d, const char *name )
{
	struct diskfs_block *b = page_alloc(0);
	int i, j;

	int nblocks = d->size / DISKFS_BLOCK_SIZE;
	if(d->size%DISKFS_BLOCK_SIZE) nblocks++;

	for(i=0;i<nblocks;i++) {
		diskfs_inode_read(d,b,i);
		for(j=0;j<DISKFS_ITEMS_PER_BLOCK;j++) {
			struct diskfs_item *r = &b->items[j];
			if(r->type!=DISKFS_ITEM_BLANK && !strncmp(name,r->name,r->name_length)) {
				int inumber = r->inumber;
				page_free(b);
				return diskfs_dirent_create(d->volume,inumber,r->type);
			}
		}
	}

	page_free(b);
	return 0;
}

int diskfs_dirent_list( struct fs_dirent *d, char *buffer, int length )
{
	struct diskfs_block *b = page_alloc(0);

	int nblocks = d->size / DISKFS_BLOCK_SIZE;
	if(d->size%DISKFS_BLOCK_SIZE) nblocks++;

	int i,j;
	int total = 0;

	for(i=0;i<nblocks;i++) {
		diskfs_inode_read(d,b,i);

		for(j=0;j<DISKFS_ITEMS_PER_BLOCK;j++) {
			struct diskfs_item *r = &b->items[j];

			switch(r->type) {
				case DISKFS_ITEM_FILE:
				case DISKFS_ITEM_DIR:
					memcpy(buffer,r->name,r->name_length);
					buffer[r->name_length] = 0;
					buffer += r->name_length + 1;
					length -= r->name_length + 1;
					total += r->name_length + 1;
					break;
				case DISKFS_ITEM_BLANK:
					break;
			}
		}
	}

	page_free(b);

	return total;
}

int diskfs_dirent_resize( struct fs_dirent *d, uint32_t size )
{
	d->size = d->disk.size = size;
	return 0;
}

static int diskfs_dirent_add( struct fs_dirent *d, const char *name, int type, int inumber )
{
	struct diskfs_block *b = page_alloc(0);
	int i, j;

	int nblocks = d->size / DISKFS_BLOCK_SIZE;
	if(d->size%DISKFS_BLOCK_SIZE) nblocks++;

	for(i=0;i<nblocks;i++) {
		diskfs_inode_read(d,b,i);
		for(j=0;j<DISKFS_ITEMS_PER_BLOCK;j++) {
			struct diskfs_item *r = &b->items[j];
			if(r->type==DISKFS_ITEM_BLANK) {

				r->type = type;
				r->inumber = inumber;
				r->name_length = strlen(name);
				memcpy(r->name,name,r->name_length);
				diskfs_inode_write(d,b,i);
				page_free(b);
				return 0;
			}
		}
	}

	memset(b->data,0,DISKFS_BLOCK_SIZE);
	struct diskfs_item *r = &b->items[0];

	r->inumber = inumber;
	r->type = type;
	r->name_length = strlen(name);
	memcpy(r->name,name,r->name_length);

	diskfs_dirent_resize(d,d->size+sizeof(*r));
	diskfs_inode_write(d,b,i);
	diskfs_inode_save(d->volume,d->inumber,&d->disk);

       	page_free(b);
	return 0;
}

struct fs_dirent * diskfs_dirent_create_file_or_dir( struct fs_dirent *d, const char *name, int type )
{
	struct fs_dirent *t = diskfs_dirent_lookup(d,name);
	if(t) {
		diskfs_dirent_close(t);
		return 0;
	}

	int inumber = diskfs_inumber_alloc(d->volume);
	if(inumber==0) return 0; // KERROR_OUT_OF_SPACE

	struct diskfs_inode inode;
	memset(&inode,0,sizeof(inode));
	inode.inuse = 1;
	inode.size = 0;
	diskfs_inode_save(d->volume,inumber,&inode);
	diskfs_dirent_add(d,name,type,inumber);
	return diskfs_dirent_create(d->volume,inumber,type);
}

struct fs_dirent * diskfs_dirent_create_file( struct fs_dirent *d, const char *name )
{
	return diskfs_dirent_create_file_or_dir(d,name,DISKFS_ITEM_FILE);
}

struct fs_dirent * diskfs_dirent_create_dir( struct fs_dirent *d, const char *name )
{
	return diskfs_dirent_create_file_or_dir(d,name,DISKFS_ITEM_DIR);
}

void diskfs_inode_delete( struct fs_volume *v, struct diskfs_inode *node, int inumber )
{
	int size = 0;
	int i;


	// XXX check for errors in here
	for(i=0;i<DISKFS_DIRECT_POINTERS;i++) {
		diskfs_data_block_free(v,node->direct[i]);
		size += v->block_size;
		if(size>=node->size) break;
	}

	if(size<node->size) {
		struct diskfs_block *b = page_alloc(0);
		diskfs_data_block_read(v,b,node->indirect);
		for(i=0;i<DISKFS_POINTERS_PER_BLOCK;i++) {
			diskfs_data_block_free(v,b->pointers[i]);
			size += v->block_size;
			if(size>=node->size) break;
		}
		page_free(b);
	}

	memset(node,sizeof(*node),0);
	diskfs_inode_save(v,inumber,node);
	diskfs_inumber_free(v,inumber);
}

int diskfs_dirent_remove( struct fs_dirent *d, const char *name )
{
	struct diskfs_block *b = page_alloc(0);

	int name_length = strlen(name);

	int i, j;
	int nblocks = d->size / DISKFS_BLOCK_SIZE;
	if(d->size%DISKFS_BLOCK_SIZE) nblocks++;

	for(i=0;i<nblocks;i++) {
		diskfs_inode_read(d,b,i);
		for(j=0;j<DISKFS_ITEMS_PER_BLOCK;j++) {
			struct diskfs_item *r = &b->items[j];

			if(r->type!=DISKFS_ITEM_BLANK && r->name_length==name_length && !strncmp(name,r->name,name_length)) {

				if(r->type==DISKFS_ITEM_DIR) {
					struct diskfs_inode inode;
					diskfs_inode_load(d->volume,r->inumber,&inode);
					if(inode.size>0) {
						page_free(b);
						return KERROR_NOT_EMPTY;
					}
				}

				int inumber = r->inumber;
				r->type = DISKFS_ITEM_BLANK;
				diskfs_inode_write(d,b,i);
				diskfs_inode_delete(d->volume,&d->disk,inumber);
				page_free(b);
				return 0;
			}
		}
	}

	return KERROR_NOT_FOUND;
}

int diskfs_dirent_write_block( struct fs_dirent *d, const char *data, uint32_t blockno )
{
	return diskfs_inode_write(d,(void*)data,blockno);
}

int diskfs_dirent_read_block( struct fs_dirent *d, char *data, uint32_t blockno )
{
	return diskfs_inode_read(d,(void*)data,blockno);
}

extern struct fs disk_fs;

struct fs_volume * diskfs_volume_open( struct device *device )
{
	struct diskfs_block *b = page_alloc(0);

	printf("diskfs: opening device %s unit %d\n",device_name(device),device_unit(device));

	diskfs_block_read(device,b,0);

	struct diskfs_superblock *sb = &b->superblock;

	if(sb->magic!=DISKFS_MAGIC) {
		printf("diskfs: no filesystem found!\n");
		page_free(b);
		return 0;
	}

       	struct fs_volume *v = kmalloc(sizeof(*v));
	v->fs = &disk_fs;
	v->device = device;
	v->block_size = device_block_size(device);
	v->refcount = 1;
	v->disk = *sb;

	page_free(b);

	printf("diskfs: %d bitmap blocks, %d inode blocks, %d data blocks\n",
		v->disk.bitmap_blocks,
		v->disk.inode_blocks,
		v->disk.data_blocks);

	return v;
}

struct fs_dirent * diskfs_volume_root( struct fs_volume *v )
{
	return diskfs_dirent_create(v,0,DISKFS_ITEM_DIR);
}

int diskfs_volume_close( struct fs_volume *v )
{
	return 0;
}

int diskfs_volume_format( struct device *device )
{
	struct diskfs_block *b = page_alloc(1);
	struct diskfs_superblock sb;

	int nblocks = device_nblocks(device);

	printf("diskfs: formatting device %s unit %d\n",device_name(device),device_unit(device));

	sb.magic = DISKFS_MAGIC;
	sb.block_size = DISKFS_BLOCK_SIZE;
	sb.inode_blocks = 1024 / sizeof(struct diskfs_inode);

	int remaining_blocks = nblocks - sb.inode_blocks;
	sb.bitmap_blocks = 1 + remaining_blocks / (DISKFS_BLOCK_SIZE*8);
	sb.data_blocks = remaining_blocks - sb.bitmap_blocks;

	sb.inode_start = 1;
	sb.bitmap_start = sb.inode_start + sb.inode_blocks;
	sb.data_start = sb.bitmap_start + sb.bitmap_blocks;

	printf("diskfs: %d inode blocks, %d bitmap blocks, %d data blocks\n",
	       sb.inode_blocks, sb.bitmap_blocks, sb.data_blocks );

	memset(b,0,DISKFS_BLOCK_SIZE);
	b->superblock = sb;

	printf("diskfs: writing superblock\n");
	diskfs_block_write(device,b,0);

	memset(b,0,DISKFS_BLOCK_SIZE);

	int i;

	printf("diskfs: writing %d inode blocks\n",sb.inode_blocks);

	for(i=sb.inode_blocks-1;i>=0;i--) {
		diskfs_block_write(device,b,sb.inode_start+i);
	}

	printf("diskfs: writing %d bitmap blocks\n",sb.bitmap_blocks);

	for(i=sb.bitmap_blocks-1;i>=0;i--) {
		diskfs_block_write(device,b,sb.bitmap_start+i);
	}

	printf("diskfs: creating root directory\n");

	// Mark the zeroth and first blocks as used.
	b->data[0] = 0x03;
	diskfs_block_write(device,b,sb.bitmap_start);

	// Set up the zeroth inode as the root directory with a single direct block.
	memset(b,0,DISKFS_BLOCK_SIZE);
	b->inodes[0].inuse = 1;
	b->inodes[0].size = sizeof(struct diskfs_item);
	b->inodes[0].direct[0] = 1;
	diskfs_block_write(device,b,sb.inode_start);

	// Create the first directory entry as dot and write it to the first block.
	memset(b,0,DISKFS_BLOCK_SIZE);
	b->items[0].inumber = 0;
	b->items[0].type = DISKFS_ITEM_DIR;
	b->items[0].name_length = 1;
	b->items[0].name[0] = '.';
	diskfs_block_write(device,b,sb.data_start+1);

	page_free(b);

	printf("diskfs: flushing buffer cache\n");
	bcache_flush_device(device);

	return 0;
}

struct fs_ops diskfs_ops = {
	.volume_open = diskfs_volume_open,
	.volume_close = diskfs_volume_close,
	.volume_format = diskfs_volume_format,
	.volume_root = diskfs_volume_root,

	.lookup = diskfs_dirent_lookup,
	.mkdir = diskfs_dirent_create_dir,
	.mkfile = diskfs_dirent_create_file,
	.read_block = diskfs_dirent_read_block,
	.write_block = diskfs_dirent_write_block,
	.list = diskfs_dirent_list,
	.remove = diskfs_dirent_remove,
	.resize = diskfs_dirent_resize,
	.close = diskfs_dirent_close
};


struct fs disk_fs = {
	"diskfs",
	&diskfs_ops,
	0
};

int diskfs_init(void)
{
	fs_register(&disk_fs);
	return 0;
}

