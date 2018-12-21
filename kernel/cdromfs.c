/*
Copyright (C) 2016 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/

#include "kmalloc.h"
#include "console.h"
#include "kernel/types.h"
#include "string.h"
#include "iso9660.h"
#include "ata.h"
#include "memory.h"
#include "fs.h"
#include "device.h"
#include "bcache.h"
#include "cdromfs.h"
#include "kernel/syscall.h"

struct cdrom_volume {
	struct device *device;
	int root_sector;
	int root_length;
	int total_sectors;
};

struct cdrom_dirent {
	struct cdrom_volume *volume;
	int sector;
	int length;
	int isdir;
};

static struct fs_volume *cdrom_volume_as_volume(struct cdrom_volume *cdv);
static struct fs_dirent *cdrom_dirent_as_dirent(struct cdrom_dirent *cdd);

static struct cdrom_dirent *cdrom_dirent_create(struct cdrom_volume *volume, int sector, int length, int isdir)
{
	struct cdrom_dirent *d = kmalloc(sizeof(*d));
	if(!d)
		return 0;

	d->volume = volume;
	d->sector = sector;
	d->length = length;
	d->isdir = isdir;
	return d;
}

static int cdrom_dirent_read_block(struct fs_dirent *d, char *buffer, uint32_t blocknum)
{
	struct cdrom_dirent *cdd = d->private_data;
	int nblocks = bcache_read(cdd->volume->device, buffer, 1, (int) cdd->sector + blocknum);
	if(nblocks == 1) {
		return ATAPI_BLOCKSIZE;
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
	struct cdrom_dirent *cddir = dir->private_data;
	if(!cddir->isdir) return 0;

	char *temp = memory_alloc_page(0);
	if(!temp) return 0;

	int nsectors = cddir->length / ATAPI_BLOCKSIZE + (cddir->length % ATAPI_BLOCKSIZE ? 1 : 0);

	int i;
	for(i=0;i<nsectors;i++) {
		cdrom_dirent_read_block(dir,temp,i);

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
				struct cdrom_dirent *r;
				r = cdrom_dirent_create(
					cddir->volume,
					d->first_sector_little,
					d->length_little,
					d->flags & ISO_9660_EXTENT_FLAG_DIRECTORY);
				memory_free_page(temp);
				return cdrom_dirent_as_dirent(r);
			}
			d = (struct iso_9660_directory_entry *) ((char *) d + d->descriptor_length);
		}
	}

	memory_free_page(temp);

	return 0;
}

static int cdrom_dirent_read_dir(struct fs_dirent *dir, char *buffer, int buffer_length)
{
	struct cdrom_dirent *cddir = dir->private_data;
	if(!cddir->isdir) return KERROR_NOT_A_DIRECTORY;

	char *temp = memory_alloc_page(0);
	if(!temp) return KERROR_OUT_OF_MEMORY;

	int nsectors = cddir->length / ATAPI_BLOCKSIZE + (cddir->length % ATAPI_BLOCKSIZE ? 1 : 0);
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

	memory_free_page(temp);

	return total;
}

static int cdrom_dirent_close(struct fs_dirent *d)
{
	struct cdrom_dirent *cdd = d->private_data;
	kfree(cdd);
	kfree(d);
	return 0;
}

static struct fs_dirent *cdrom_volume_root(struct fs_volume *v)
{
	struct cdrom_volume *cdv = v->private_data;
	struct cdrom_dirent *cdd = cdrom_dirent_create(cdv, cdv->root_sector, cdv->root_length, 1);
	return cdrom_dirent_as_dirent(cdd);
}

static struct fs_volume *cdrom_volume_open( struct device *device )
{
	struct cdrom_volume *cdv = kmalloc(sizeof(*cdv));
	if(!cdv)
		return 0;

	struct iso_9660_volume_descriptor *d = memory_alloc_page(0);
	if(!d) {
		kfree(cdv);
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
			cdv->root_sector = d->root.first_sector_little;
			cdv->root_length = d->root.length_little;
			cdv->total_sectors = d->nsectors_little;
			cdv->device = device;

			printf("cdromfs: mounted filesystem on %s-%d\n", device_name(cdv->device), device_unit(cdv->device));

			memory_free_page(d);

			return cdrom_volume_as_volume(cdv);

		} else if(d->type == ISO_9660_VOLUME_TYPE_TERMINATOR) {
			break;
		} else {
			continue;
		}
	}

	printf("cdromfs: no filesystem found\n");
	return 0;
}

static int cdrom_volume_close(struct fs_volume *v)
{
	struct cdrom_volume *cdv = v->private_data;
	printf("cdromfs: unmounted filesystem from %s-%d\n", device_name(cdv->device), device_unit(cdv->device));
	kfree(v);
	return 0;
}

const static struct fs_ops cdrom_ops = {
	.mount = cdrom_volume_open,
	.umount = cdrom_volume_close,
	.root = cdrom_volume_root,
	.close = cdrom_dirent_close,
	.readdir = cdrom_dirent_read_dir,
	.lookup = cdrom_dirent_lookup,
	.mkdir = 0,
	.rmdir = 0,
	.link = 0,
	.unlink = 0,
	.read_block = cdrom_dirent_read_block,
	.write_block = 0,
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

static struct fs_volume *cdrom_volume_as_volume(struct cdrom_volume *cdv)
{
	struct fs_volume *v = kmalloc(sizeof(struct fs_volume));
	memset(v, 0, sizeof(struct fs_volume));
	v->refcount = 1;
	v->private_data = cdv;
	v->fs->ops = &cdrom_ops;
	v->block_size = ATAPI_BLOCKSIZE;
	return v;
}

static struct fs_dirent *cdrom_dirent_as_dirent(struct cdrom_dirent *cdd)
{
	struct fs_dirent *d = kmalloc(sizeof(struct fs_dirent));
	memset(d, 0, sizeof(struct fs_volume));
	d->refcount = 1;
	d->private_data = cdd;
	d->size = cdd->length;
	d->isdir = cdd->isdir;
	return d;
}
