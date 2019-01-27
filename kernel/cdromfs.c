/*
Copyright (C) 2016-2019 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/

#include "kmalloc.h"
#include "kernel/types.h"
#include "kernel/error.h"
#include "string.h"
#include "page.h"
#include "fs.h"
#include "fs_internal.h"
#include "cdromfs.h"
#include "iso9660.h"
#include "device.h"
#include "bcache.h"

static struct fs_dirent *cdrom_dirent_create(struct fs_volume *volume, int sector, int length, int isdir)
{
	struct fs_dirent *d = kmalloc(sizeof(*d));
	if(!d) return 0;

	d->volume = volume;
	d->refcount = 1;
	d->size = length;
	d->isdir = isdir;
	d->cdrom.sector = sector;

	return d;
}

static int cdrom_dirent_read_block(struct fs_dirent *d, char *buffer, uint32_t blocknum)
{
	int nblocks = bcache_read(d->volume->device, buffer, 1, d->cdrom.sector + blocknum);
	if(nblocks == 1) {
		return CDROMFS_BLOCK_SIZE;
	} else {
		return -1;
	}
}

static void fix_filename(char *name, int length)
{
	// Plain files typically end with a semicolon and version, remove it.
	if(length > 2 && name[length - 2] == ';') {
		length -= 2;
	}
	// Files without a suffix end with a dot, remove it.
	if(length > 1 && name[length - 1] == '.') {
		length--;
	}
	// In any case, null-terminate the string
	name[length] = 0;

	// And make it lowercase
	strtolower(name);
}

static struct fs_dirent *cdrom_dirent_lookup(struct fs_dirent *dir, const char *name)
{
	if(!dir->isdir) return 0;

	char *temp = page_alloc(0);
	if(!temp) return 0;

	int nsectors = dir->size / CDROMFS_BLOCK_SIZE + (dir->size % CDROMFS_BLOCK_SIZE ? 1 : 0);

	int i;
	for(i=0;i<nsectors;i++) {
		cdrom_dirent_read_block(dir,temp,i);
		// XXX check result here!

		struct iso_9660_directory_entry *d = (struct iso_9660_directory_entry *) temp;

		while(d->descriptor_length > 0) {

			const char *dname;
			int dname_length;

			if(d->ident[0] == 0) {
				dname = ".";
				dname_length = 2;
			} else if(d->ident[0] == 1) {
				dname = "..";
				dname_length = 3;
			} else {
				fix_filename(d->ident, d->ident_length);
				dname = d->ident;
				dname_length = strlen(dname) + 1;
			}

			if(!strncmp(name,dname,dname_length)) {
				struct fs_dirent *r;
				r = cdrom_dirent_create(
					dir->volume,
					d->first_sector_little,
					d->length_little,
					d->flags & ISO_9660_EXTENT_FLAG_DIRECTORY);
				page_free(temp);
				return r;
			}
			d = (struct iso_9660_directory_entry *) ((char *) d + d->descriptor_length);
		}
	}

	page_free(temp);

	return 0;
}

static int cdrom_dirent_close( struct fs_dirent *d )
{
	return 0;
}

static int cdrom_dirent_list(struct fs_dirent *dir, char *buffer, int buffer_length)
{
	if(!dir->isdir) return KERROR_NOT_A_DIRECTORY;

	char *temp = page_alloc(0);
	if(!temp) return KERROR_OUT_OF_MEMORY;

	int nsectors = dir->size / CDROMFS_BLOCK_SIZE + (dir->size % CDROMFS_BLOCK_SIZE ? 1 : 0);
	int total = 0;

	int i;
	for(i=0;i<nsectors;i++) {
		cdrom_dirent_read_block(dir,temp,i);

		struct iso_9660_directory_entry *d = (struct iso_9660_directory_entry *) temp;

		while(d->descriptor_length > 0 && buffer_length > 0) {

			const char *dname;
			int dname_length;

			if(d->ident[0] == 0) {
				dname = ".";
				dname_length = 2;
			} else if(d->ident[0] == 1) {
				dname = "..";
				dname_length = 3;
			} else {
				fix_filename(d->ident, d->ident_length);
				dname = d->ident;
				dname_length = strlen(dname) + 1;
			}

			// If there is enough space, keep copying items.
			// If not, count them up to return the value.

			if (buffer_length > dname_length) {
				strcpy(buffer,dname);
				buffer += dname_length;
				buffer_length -= dname_length;
			}

			total += dname_length;

			d = (struct iso_9660_directory_entry *) ((char *) d + d->descriptor_length);
		}
	}

	page_free(temp);

	return total;
}

static struct fs_volume *cdrom_volume_create( struct device *device )
{
	struct fs_volume *v = kmalloc(sizeof(*v));
	if(!v) return 0;

	memset(v, 0, sizeof(struct fs_volume));
	v->device = device;
	v->refcount = 1;
	v->block_size = device_block_size(device);

	return v;
}

static int cdrom_volume_close(struct fs_volume *v)
{
	return 0;
}

static struct fs_volume *cdrom_volume_open( struct device *device )
{
	struct fs_volume *v = cdrom_volume_create(device);

	struct iso_9660_volume_descriptor *d = page_alloc(0);
	if(!d) {
		kfree(v);
		return 0;
	}

	printf("cdromfs: scanning %s unit %d...\n",device_name(device),device_unit(device));

	int j;

	for(j = 0; j < 16; j++) {
		printf("cdromfs: checking volume %d\n", j);

		bcache_read(device, (char*)d, 1, j + 16);
		// XXX check reuslt

		if(strncmp(d->magic, "CD001", 5))
			continue;

		if(d->type == ISO_9660_VOLUME_TYPE_PRIMARY) {
			v->cdrom.root_sector = d->root.first_sector_little;
			v->cdrom.root_length = d->root.length_little;
			v->cdrom.total_sectors = d->nsectors_little;
			v->device = device;

			printf("cdromfs: mounted filesystem on %s-%d\n", device_name(v->device), device_unit(v->device));

			page_free(d);

			return v;

		} else if(d->type == ISO_9660_VOLUME_TYPE_TERMINATOR) {
			break;
		} else {
			continue;
		}
	}

	page_free(d);
	cdrom_volume_close(v);

	printf("cdromfs: no filesystem found\n");
	return 0;
}

static struct fs_dirent *cdrom_volume_root(struct fs_volume *v)
{
	return cdrom_dirent_create(v,v->cdrom.root_sector,v->cdrom.root_length, 1);
}

const static struct fs_ops cdrom_ops = {
	.volume_open = cdrom_volume_open,
	.volume_close = cdrom_volume_close,
	.volume_root = cdrom_volume_root,

	.lookup = cdrom_dirent_lookup,
	.mkdir = 0,
	.mkfile = 0,
	.read_block = cdrom_dirent_read_block,
	.write_block = 0,
	.list = cdrom_dirent_list,
	.remove = 0,
	.resize = 0,
	.close = cdrom_dirent_close,
};

static struct fs cdrom_fs = {
	"cdromfs",
	&cdrom_ops,
	0
};

int cdrom_init()
{
	fs_register(&cdrom_fs);
	return 0;
}
