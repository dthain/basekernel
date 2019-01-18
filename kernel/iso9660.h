/*
Copyright (C) 2016-2019 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/

#ifndef ISO9660_H
#define ISO9660_H

#include "kernel/types.h"

#pragma pack(1)

struct iso_9660_directory_entry {
	uint8_t descriptor_length;
	uint8_t extended_sectors;
	uint32_t first_sector_little;
	uint32_t first_sector_big;
	uint32_t length_little;
	uint32_t length_big;
	uint8_t year;
	uint8_t month;
	uint8_t mday;
	uint8_t hour;
	uint8_t minute;
	uint8_t second;
	uint8_t timezone;
	uint8_t flags;
	uint8_t unit_size;
	uint8_t interleave_gap;
	uint16_t volume_sequence_little;
	uint16_t volume_sequence_big;
	uint8_t ident_length;
	char ident[1];
};

#define ISO_9660_EXTENT_FLAG_HIDDEN     1
#define ISO_9660_EXTENT_FLAG_DIRECTORY  2

struct iso_9660_time {
	char year[4];
	char month[2];
	char mday[2];
	char hour[2];
	char minute[2];
	char second[2];
	char subsec[2];
	char timezone;
};

#define ISO_9660_VOLUME_TYPE_BOOT 0
#define ISO_9660_VOLUME_TYPE_PRIMARY 1
#define ISO_9660_VOLUME_TYPE_SUPPLEMENTARY 2
#define ISO_9660_VOLUME_TYPE_PARTITION 3
#define ISO_9660_VOLUME_TYPE_TERMINATOR 255

struct iso_9660_volume_descriptor {
	uint8_t type;
	char magic[5];
	char other[2];
	char system[32];
	char volume[32];
	char reserved1[8];
	uint32_t nsectors_little;
	uint32_t nsectors_big;
	char reserved2[32];
	uint16_t volume_set_size_little;
	uint16_t volume_set_size_big;
	uint16_t volume_sequence_number_little;
	uint16_t volume_sequence_number_big;
	uint16_t sector_size_little;
	uint16_t sector_size_big;
	uint32_t path_table_size_little;
	uint32_t path_table_size_big;
	uint32_t first_path_table_start_little;
	uint32_t second_path_table_start_little;
	uint32_t first_path_table_start_big;
	uint32_t second_path_table_start_big;
	struct iso_9660_directory_entry root;
	char volume_set[128];
	char publisher[128];
	char preparer[128];
	char application[128];
	char copyright_file[37];
	char abstract_file[37];
	char bibliography_file[37];
	struct iso_9660_time creation_time;
	struct iso_9660_time modify_time;
	struct iso_9660_time expire_time;
	struct iso_9660_time effective_time;
	char unknown[2];
};

#endif
