/*
Copyright (C) 2016 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/

#include "kmalloc.h"
#include "console.h"
#include "kerneltypes.h"
#include "string.h"
#include "iso9660.h"
#include "ata.h"
#include "memory.h"
#include "fs.h"
#include "cdromfs.h"

struct cdrom_volume {
	int unit;
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

struct cdrom_file {
	struct cdrom_dirent *dirent;
	char *buffer;
	uint32_t block;
	uint32_t offset;
	uint32_t buffer_size;
};

int cdrom_init() {
	char cdrom_name[] = "cdrom";
	char *cdrom_name_cpy = kmalloc(6);
	struct fs f;
	strcpy(cdrom_name_cpy, cdrom_name);
	fs_register(&f);
	return 0;
}

static struct volume *cdrom_volume_as_volume(struct cdrom_volume *cdv) {
	struct volume *v = kmalloc(sizeof(struct volume));
	memset(v, 0, sizeof(struct volume));
	v->private_data = cdv;
	return v;
}

static struct dirent *cdrom_dirent_as_dirent(struct cdrom_dirent *cdd) {
	struct dirent *d = kmalloc(sizeof(struct dirent));
	memset(d, 0, sizeof(struct volume));
	d->private_data = cdd;
	d->sz = cdd->length;
	return d;
}

static struct file *cdrom_file_as_file(struct cdrom_file *cdf) {
	struct file *f = kmalloc(sizeof(struct file));
	memset(f, 0, sizeof(struct file));
	f->private_data = cdf;
	return f;
}

static struct cdrom_dirent * cdrom_dirent_create( struct cdrom_volume *volume, int sector, int length, int isdir )
{
	struct cdrom_dirent *d = kmalloc(sizeof(*d));
	if(!d) return 0;

	d->volume = volume;
	d->sector = sector;
	d->length = length;
	d->isdir = isdir;
	return d;
}

static struct cdrom_file *cdrom_file_create( struct cdrom_dirent *d )
{
	struct cdrom_file *f = kmalloc(sizeof(struct cdrom_file));
	memset(f, 0, sizeof(struct cdrom_file));
	f->buffer = kmalloc(ATAPI_BLOCKSIZE);
	f->dirent = d;
	return f;
}

static void cdrom_file_dealloc( struct cdrom_file *f )
{
	kfree(f->buffer);
	kfree(f);
}

static char * cdrom_dirent_load( struct dirent *d )
{
	struct cdrom_dirent *cdd = d->private_data;
	int nsectors = cdd->length/ATAPI_BLOCKSIZE + (cdd->length&ATAPI_BLOCKSIZE)?1:0;
	char *data = kmalloc(nsectors*ATAPI_BLOCKSIZE);
	if(!data) return 0;

	atapi_read(cdd->volume->unit,data,nsectors,cdd->sector);
	// XXX check result

	return data;
}

static void fix_filename( char *name, int length )
{
	if(length>2) {
		length -= 2;
		name[length] = 0;
	}

	if(length>1 && name[length-1]=='.') {
		length--;
		name[length] = 0;
	}

}

/*
Read an entire cdrom file into the target address.
*/

static int  cdrom_dirent_read_block( struct cdrom_dirent *cdd, char *buffer, int blocknum )
{
	return atapi_read( cdd->volume->unit, buffer, 1, cdd->sector + blocknum );
}

#if 0
/*
Read an entire cdrom file into the target address.
*/

int cdrom_dirent_readfile( struct cdrom_dirent *d, char *data, int length )
{
	int nsectors = d->length/ATAPI_BLOCKSIZE;
	int remainder = d->length%ATAPI_BLOCKSIZE;

	if(length<d->length) length = d->length;

	atapi_read( d->volume->unit, data, nsectors, d->sector);
	// check success here

	if(remainder>0) {
		char *page = memory_alloc_page(0);
		// XXX check success here
		atapi_read(d->volume->unit,page,1,d->sector+nsectors);
		memcpy(data + ATAPI_BLOCKSIZE*nsectors,page,remainder);
		memory_free_page(page);
	}

	return length;
}
#endif

struct dirent * cdrom_dirent_namei( struct dirent *d, const char *path )
{
	char *lpath = kmalloc(strlen(path)+1);
	strcpy(lpath,path);

	char *part = strtok(lpath,"/");
	while(part) {
		d = cdrom_dirent_lookup(d,part);
		if(!d) break;

		part = strtok(0,"/");
	}
	kfree(lpath);
	return d;
}

struct dirent * cdrom_dirent_lookup( struct dirent *dir, const char *name )
{
	struct cdrom_dirent *cddir = dir->private_data;
	char *data = cdrom_dirent_load(dir);
	if(!data) return 0;

	int data_length = cddir->length;

	struct iso_9660_directory_entry *d = (struct iso_9660_directory_entry *) data;

	while(data_length>0 && d->descriptor_length>0 ) {
		fix_filename(d->ident,d->ident_length);

		if(!strcmp(name,d->ident)) {
			struct cdrom_dirent *r = cdrom_dirent_create(
				cddir->volume,
				d->first_sector_little,
				d->length_little,
				d->flags & ISO_9660_EXTENT_FLAG_DIRECTORY );

			kfree(data);
			return cdrom_dirent_as_dirent(r);
		}

		d = (struct iso_9660_directory_entry *)((char*)d+d->descriptor_length);
		data_length -= d->descriptor_length;
	}

	kfree(data);

	return 0;
}

int cdrom_dirent_read_dir( struct dirent *dir, char *buffer, int buffer_length )
{
	struct cdrom_dirent *cddir = dir->private_data;
	char *data = cdrom_dirent_load(dir);
	if(!data) return 0;

	int data_length = cddir->length;
	int total = 0;

	struct iso_9660_directory_entry *d = (struct iso_9660_directory_entry *) data;

	while(data_length>0 && d->descriptor_length>0 ) {
		fix_filename(d->ident,d->ident_length);

		if(d->ident[0]==0) {
			strcpy(buffer,".");
			buffer+=2;
			buffer_length-=2;
			total+=2;
		} else if(d->ident[0]==1) {
			strcpy(buffer,"..");
			buffer+=3;
			buffer_length-=3;
			total+=3;
		} else {
			strcpy(buffer,d->ident);
			int len = strlen(d->ident) + 1;
			buffer += len;
			buffer_length -= len;
			total += len;
		}
		d = (struct iso_9660_directory_entry *)((char*)d+d->descriptor_length);
		data_length -= d->descriptor_length;
	}

	kfree(data);

	return total;
}

void cdrom_dirent_close( struct dirent *d )
{
	struct cdrom_dirent *cdd = d->private_data;
	kfree(cdd);
	kfree(d);
}

struct dirent * cdrom_volume_root( struct volume *v )
{
	struct cdrom_volume *cdv = v->private_data;
	struct cdrom_dirent *cdd = cdrom_dirent_create(cdv,cdv->root_sector,cdv->root_length,1);
	return cdrom_dirent_as_dirent(cdd);
}

struct volume * cdrom_volume_open( uint32_t unit )
{
	struct cdrom_volume *cdv = kmalloc(sizeof(*cdv));
	if(!cdv) return 0;

	struct iso_9660_volume_descriptor *d = memory_alloc_page(0);
	if(!d) {
		kfree(cdv);
		return 0;
	}

	printf("cdromfs: scanning atapi unit %d...\n",unit);

	int j;
	for(j=0;j<16;j++) {
		printf("cdromfs: checking volume %d\n",j);

		atapi_read(unit,d,1,j+16);
		// XXX check reuslt

		if(strncmp(d->magic,"CD001",5)) continue;

		if(d->type==ISO_9660_VOLUME_TYPE_PRIMARY) {
			cdv->root_sector = d->root.first_sector_little;
			cdv->root_length = d->root.length_little;
			cdv->total_sectors = d->nsectors_little;
			cdv->unit = unit;

			printf("cdromfs: mounted filesystem on unit %d\n",cdv->unit);

			memory_free_page(d);

			return cdrom_volume_as_volume(cdv);

		} else if(d->type==ISO_9660_VOLUME_TYPE_TERMINATOR) {
			break;
		} else {
			continue;
		}
	}

	console_printf("cdromfs: no filesystem found\n");
	return 0;		
}

void cdrom_volume_close( struct volume *v )
{
	struct cdrom_volume *cdv = v->private_data;
	console_printf("cdromfs: umounted filesystem from unit %d\n",cdv->unit);
	kfree(v);
}



int cdrom_file_read(struct file *f, char *buffer, uint32_t n)
{
	struct cdrom_file *cdf = f->private_data;
	struct cdrom_dirent *cdd = cdf->dirent;
	uint32_t read = 0;
	while (n > 0 && cdf->block * ATAPI_BLOCKSIZE + cdf->offset < cdd->length) {
		uint32_t to_read = 0;
		if (cdf->offset == cdf->buffer_size) {
			cdrom_dirent_read_block(cdd, cdf->buffer, cdf->block);
			if (cdd->length <= (cdf->block + 1) * ATAPI_BLOCKSIZE) {
				cdf->buffer_size = cdd->length - cdf->block * ATAPI_BLOCKSIZE;
			} else {
				cdf->buffer_size = ATAPI_BLOCKSIZE;
			}
			cdf->block++;
			cdf->offset = 0;
		}
		to_read = cdf->buffer_size - cdf->offset;
		if (n < cdf->buffer_size - cdf->offset) {
			to_read = n;
		}
		memcpy(buffer + read, cdf->buffer + cdf->offset, to_read);
		cdf->offset += to_read;
		n -= to_read;
		read += to_read;
	}
	return read;
}

struct file *cdrom_file_open(struct dirent *d, int8_t mode)
{
	struct cdrom_dirent *cdd = d->private_data;
	struct cdrom_file *cdf = cdrom_file_create(cdd);
	return cdrom_file_as_file(cdf);
}
