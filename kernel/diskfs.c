/*
Copyright (C) 2019 The University of Notre Dame
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
#include "memory.h"

/* Read or write a block from the raw device, starting from zero. */

static int diskfs_block_read(struct fs_volume *v, struct diskfs_block *b, uint32_t blockno )
{
	return bcache_read(v->device, b->data, 1, blockno) ? DISKFS_BLOCK_SIZE : -1;
}

static int diskfs_block_write(struct fs_volume *v, struct diskfs_block *b, uint32_t blockno )
{
	return bcache_write(v->device, b->data, 1, blockno) ? DISKFS_BLOCK_SIZE : -1;
}

/* Read or write a bitmap block, starting from the bitmap offset. */

static int diskfs_bitmap_block_read(struct fs_volume *v, struct diskfs_block *b, uint32_t blockno )
{
  // XXX add protections for ranges
	return diskfs_block_read(v,b,v->disk.bitmap_start+blockno);
}

static int diskfs_bitmap_block_write(struct fs_volume *v, struct diskfs_block *b, uint32_t blockno )
{
	return diskfs_block_write(v,b,v->disk.bitmap_start+blockno);
}

/* Read or write an inode block, starting from the inode block offset. */

static int diskfs_inode_block_read(struct fs_volume *v, struct diskfs_block *b, uint32_t blockno )
{
	return diskfs_block_read(v,b,v->disk.inode_start+blockno);
}

static int diskfs_inode_block_write(struct fs_volume *v, struct diskfs_block *b, uint32_t blockno )
{
	return diskfs_block_write(v,b,v->disk.inode_start+blockno);
}

/* Read or write a data block, starting from the data block offset. */

static int diskfs_data_block_read(struct fs_volume *v, struct diskfs_block *b, uint32_t blockno )
{
	return diskfs_block_read(v,b,v->disk.data_start+blockno);
}

static int diskfs_data_block_write(struct fs_volume *v, struct diskfs_block *b, uint32_t blockno )
{
	return diskfs_block_write(v,b,v->disk.data_start+blockno);
}

/*
Allocate a new data block by scanning the bitmap.
If available, return the block number.
If nothing available, return zero.
*/

static uint32_t diskfs_data_block_alloc( struct fs_volume *v )
{
	struct diskfs_block *b = memory_alloc_page(0);
	struct diskfs_superblock *s= &v->disk;
	int i, j, k;

	for(i=0;i<s->bitmap_blocks;i++) {
		diskfs_bitmap_block_read(v,b,i);
		for(j=0;j<DISKFS_BLOCK_SIZE;j++) {
			if(b->data[j]!=0xff) {
				for(k=0;k<8;k++) {
					if(!((1<<k) & b->data[j])) {
						memory_free_page(b);
						return i*DISKFS_BLOCK_SIZE+j*8+k;
					}
				}
			}		
		}
	}

	memory_free_page(b);
	return 0;
}

static void diskfs_data_block_free( struct fs_volume *v, int blockno )
{
	struct diskfs_block *b = memory_alloc_page(0);

	int bitmap_block = blockno/DISKFS_BLOCK_SIZE;
	int bitmap_byte = blockno%DISKFS_BLOCK_SIZE/8;
	int bitmap_bit = blockno%DISKFS_BLOCK_SIZE%8;

	diskfs_bitmap_block_read(v,b,bitmap_block);
	b->data[bitmap_byte] &= ~(1<<bitmap_bit);
	diskfs_bitmap_block_write(v,b,bitmap_block);

	memory_free_page(b);
}

static int diskfs_inumber_alloc( struct fs_volume *v )
{
	struct diskfs_block *b = memory_alloc_page(0);
	int i, j;

	for(i=0;i<v->disk.inode_blocks;i++) {
		diskfs_inode_block_read(v,b,i);
		for(j=0;j<DISKFS_INODES_PER_BLOCK;j++) {
			if(!b->inodes[j].inuse) {
				memory_free_page(b);
				return i * DISKFS_INODES_PER_BLOCK + j;
			}
		}
	}

	memory_free_page(b);
	return 0;
}

static void diskfs_inumber_free( struct fs_volume *v, int inumber )
{
	int inode_block = inumber / DISKFS_INODES_PER_BLOCK;
	struct diskfs_block *b = memory_alloc_page(0);
	diskfs_inode_block_read(v,b,inode_block);
	b->inodes[inumber%DISKFS_INODES_PER_BLOCK].inuse = 0;
	diskfs_inode_block_write(v,b,inode_block);
	memory_free_page(b);
}

int diskfs_inode_load( struct fs_volume *v, int inumber, struct diskfs_inode *inode )
{
	struct diskfs_block *b = memory_alloc_page(0);

	int inode_block = inumber / DISKFS_INODES_PER_BLOCK;
	int inode_position = inumber % DISKFS_INODES_PER_BLOCK;

	diskfs_inode_block_read(v,b,inode_block);
	memcpy(inode,&b->data[inode_position],sizeof(*inode));

	memory_free_page(b);

	return 1;
}

int diskfs_inode_save( struct fs_volume *v, int inumber, struct diskfs_inode *inode )
{
	struct diskfs_block *b = memory_alloc_page(0);

	int inode_block = inumber / DISKFS_INODES_PER_BLOCK;
	int inode_position = inumber % DISKFS_INODES_PER_BLOCK;

	diskfs_inode_block_read(v,b,inode_block);
	memcpy(&b->data[inode_position],inode,sizeof(*inode));
	diskfs_inode_block_write(v,b,inode_block);

	memory_free_page(b);

	return 1;
}

int diskfs_inode_read( struct fs_dirent *d, struct diskfs_block *b, uint32_t block )
{
	int actual;

	if(block<DISKFS_DIRECT_POINTERS) {
		actual = d->disk.direct[block];
	} else {
		diskfs_data_block_read(d->v,b,d->disk.indirect);
		actual = b->pointers[block-DISKFS_DIRECT_POINTERS];
	}
		
	return diskfs_data_block_read(d->v,b,actual);
}

int diskfs_inode_write( struct fs_dirent *d, struct diskfs_block *b, uint32_t block )
{
	int actual;

	struct diskfs_inode *i = &d->disk;

	if(block<DISKFS_DIRECT_POINTERS) {
		actual = i->direct[block];
		if(actual==0) {
			actual = diskfs_data_block_alloc(d->v);
			if(actual==0) return KERROR_OUT_OF_SPACE;
			i->direct[block] = actual;
			diskfs_inode_save(d->v,d->inumber,i);	
		}
	} else {
		if(i->indirect==0) {
			actual = diskfs_data_block_alloc(d->v);
			if(actual==0) return KERROR_OUT_OF_SPACE;
			i->indirect = actual;
			diskfs_inode_save(d->v,d->inumber,i);	
			// XXX need to clear the indirect block!
		}
		diskfs_data_block_read(d->v,b,i->indirect);
		actual = b->pointers[block-DISKFS_DIRECT_POINTERS];
		// XXX what if that value is no good!?
	}

	return diskfs_data_block_write(d->v,b,actual);
}

int diskfs_inode_setsize( struct fs_dirent *d, uint32_t size )
{
	// XXX this only handles the case of getting bigger!
	d->size = d->disk.size = size;
	return 0;
}

struct fs_dirent * diskfs_dirent_create( struct fs_volume *volume, int inumber )
{
	struct fs_dirent *d = kmalloc(sizeof(*d));
	memset(d,0,sizeof(*d));

	diskfs_inode_load(volume,inumber,&d->disk);

	d->v = volume;
	d->size = d->disk.size;
	d->inumber = inumber;
	d->refcount = 1;
	d->isdir = 0; // XXX where to set this?
	return d;
}

void diskfs_dirent_close( struct fs_dirent *d )
{
	// XXX write out old node if dierty!
	kfree(d);
}

struct fs_dirent * diskfs_dirent_lookup( struct fs_dirent *d, const char *name )
{
	struct diskfs_block *b = memory_alloc_page(0);
	int i, j;

	int nblocks = d->size / DISKFS_BLOCK_SIZE;

	for(i=0;i<nblocks;i++) {
		diskfs_inode_read(d,b,i);
		for(j=0;j<DISKFS_ITEMS_PER_BLOCK;j++) {
			struct diskfs_item *r = &b->items[j];
			if(r->type!=DISKFS_ITEM_BLANK && !strncmp(name,r->name,r->name_length)) {
			  return diskfs_dirent_create(d->v,r->inumber);
			}
		}
	}

	memory_free_page(b);
	return 0;
}

int diskfs_dirent_readdir( struct fs_dirent *d, char *buffer, int length )
{
	struct diskfs_block *b = memory_alloc_page(0);

	int nblocks = d->size / DISKFS_BLOCK_SIZE;
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

	memory_free_page(b);

	return total;
}

static int diskfs_dirent_add( struct fs_dirent *d, const char *name, int type, int inumber )
{
	struct diskfs_block *b = memory_alloc_page(0);
	int i, j;

	int nblocks = d->size / DISKFS_BLOCK_SIZE;

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
				memory_free_page(b);
				return 0;
			}
		}
	}

	memset(b->data,0,DISKFS_BLOCK_SIZE);
	struct diskfs_item *r = &b->items[i];

	r->inumber = inumber;
	r->type = type;
	r->name_length = strlen(name);
	memcpy(r->name,name,r->name_length);

	diskfs_inode_write(d,b,i);
	diskfs_inode_setsize(d,d->size+DISKFS_BLOCK_SIZE);
	diskfs_inode_save(d->v,d->inumber,&d->disk);

       	memory_free_page(b);
	return 0;
}

struct fs_dirent * diskfs_dirent_create_file_or_dir( struct fs_dirent *d, const char *name, int type )
{
	struct fs_dirent *t = diskfs_dirent_lookup(d,name);
	if(t) {
		diskfs_dirent_close(t);
		return 0;
	}

	int inumber = diskfs_inumber_alloc(d->v);

	diskfs_dirent_add(d,name,DISKFS_ITEM_FILE,inumber);

	return diskfs_dirent_create(d->v,inumber);
}

struct fs_dirent * diskfs_dirent_create_file( struct fs_dirent *d, const char *name )
{
	return diskfs_dirent_create_file_or_dir(d,name,DISKFS_ITEM_FILE);
}

struct fs_dirent * diskfs_dirent_create_dir( struct fs_dirent *d, const char *name )
{
	return diskfs_dirent_create_file_or_dir(d,name,DISKFS_ITEM_DIR);
}

int diskfs_dirent_delete( struct fs_dirent *d, const char *name )
{
	struct diskfs_block *b = memory_alloc_page(0);

	int i, j;
	int nblocks = d->size / DISKFS_BLOCK_SIZE;

	for(i=0;i<nblocks;i++) {
		diskfs_inode_read(d,b,i);
		for(j=0;j<DISKFS_ITEMS_PER_BLOCK;j++) {
			struct diskfs_item *r = &b->items[j];

			if(!strncmp(name,r->name,r->name_length)) {
				int inumber = r->inumber;
				r->type = DISKFS_ITEM_BLANK;
				diskfs_inode_write(d,b,i);
				diskfs_inode_delete(d->v,inumber);
				memory_free_page(b);
				return 0;
			}
		}
	}

	return KERROR_NOT_FOUND;
}

#if 0

int diskfs_dirent_write_block( struct fs_dirent *d, const char *name )
{
}


int diskfs_dirent_read_block( struct fs_dirent *d, const char *name )
{
}


int diskfs_inode_load( fs_volume *v, int inumber, struct diskfs_inode *inode )

int diskfs_dirent_close( struct fs_dirent *d )
{
  // write inode back to disk
  // free the dirent
}

static int diskfs_format( struct device *device )
{
	struct diskfs_block *b = memory_alloc_page(1);

	int nblocks = device_nblocks(device);
	int blocksize = device_block_size(device);

	b->superblock.magic = DISKFS_MAGIC;

	ninodes = nblocks/
...
	  }

static int diskfs_write_superblock(struct device *device)
{
	uint8_t wbuffer[FS_BLOCK_SIZE];
	uint32_t num_blocks;
	int ata_blocksize;
	uint32_t superblock_num_blocks,  available_blocks, free_blocks,
		 total_inodes, total_inode_bitmap_bytes, total_block_bitmap_bytes, inode_sector_size,
		 inode_bit_sector_size, data_bit_sector_size;

	char zeros[ata_blocksize];
	memset(zeros, 0, ata_blocksize);
	struct diskfs_superblock super;
	superblock_num_blocks = CONTAINERS(sizeof(struct diskfs_superblock), FS_BLOCK_SIZE);
	available_blocks = num_blocks - superblock_num_blocks;
	free_blocks = (uint32_t) ((double) (available_blocks) / (1.0 + (double) (sizeof(struct diskfs_inode) + .125) / (4.0 * FS_BLOCK_SIZE) + .125 / (FS_BLOCK_SIZE)
				  )
		);
	total_inodes = free_blocks / 8;
	total_inode_bitmap_bytes = CONTAINERS(total_inodes, 8);
	total_block_bitmap_bytes = CONTAINERS(free_blocks, 8);
	inode_sector_size = CONTAINERS((total_inodes * sizeof(struct diskfs_inode)), FS_BLOCK_SIZE);
	inode_bit_sector_size = CONTAINERS(total_inode_bitmap_bytes, FS_BLOCK_SIZE);
	data_bit_sector_size = CONTAINERS(total_block_bitmap_bytes, FS_BLOCK_SIZE);

	super.magic = FS_MAGIC;
	super.blocksize = FS_BLOCK_SIZE;
	super.physical_blocksize = ata_blocksize;
	super.inode_bitmap_start = superblock_num_blocks;
	super.inode_start = super.inode_bitmap_start + inode_bit_sector_size;
	super.block_bitmap_start = super.inode_start + inode_sector_size;
	super.free_block_start = super.block_bitmap_start + data_bit_sector_size;
	super.num_inodes = total_inodes;
	super.num_free_blocks = free_blocks;

	memcpy(wbuffer, &super, sizeof(super));
	uint32_t counter = 0;
	printf("Writing inode bitmap...\n");
	for (uint32_t i = super.inode_bitmap_start; i < super.inode_start; i++) {
		if(!diskfs_write_block(device, i, zeros))
			return -1;
		counter++;
	}
	printf("%u blocks written\n", counter);
	counter = 0;
	printf("Writing free block bitmap...\n");
	for (uint32_t i = super.block_bitmap_start; i < super.free_block_start; i++) {
		if(!diskfs_write_block(device, i, zeros))
			return -1;
		counter++;
	}
	printf("writing superblock...\n");
	diskfs_write_block(device, 0, wbuffer);
	counter++;
	printf("%u blocks written\n", counter);
	return counter;
}

static struct fs_ops diskfs_ops = {
	.volume_open = diskfs_volume_open,
	.volume_close = diskfs_volume_close,
	.volume_format = diskfs_volume_format,
	.volume_root = diskfs_volume_root,

	.readdir = diskfs_read_dir,
	.mkdir = diskfs_mkdir,
	.mkfile = diskfs_mkfile,
	.lookup = diskfs_dirent_lookup,
	.rmdir = diskfs_rmdir,
	.unlink = diskfs_unlink,
	.link = diskfs_link,
	.read_block = diskfs_read_block,
	.write_block = diskfs_write_block,
	.resize = diskfs_dirent_resize,
};


static struct fs disk_fs = {
	"diskfs",
	&diskfs_ops,
	0
};

static int diskfs_register()
{
	fs_register(&disk_fs);
	return 0;
}

int diskfs_init(void)
{
	diskfs_register();
	return 0;
}

#endif
