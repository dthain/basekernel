/*
Copyright (C) 2015-2019 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/

#include "elf.h"
#include "fs.h"
#include "string.h"
#include "console.h"
#include "process.h"
#include "kernel/syscall.h"
#include "memorylayout.h"

struct elf_header {
	char ident[16];
	uint16_t type;
	uint16_t machine;
	uint32_t version;
	uint32_t entry;
	uint32_t program_offset;
	uint32_t section_offset;
	uint32_t flags;
	uint16_t header_size;
	uint16_t phentsize;
	uint16_t phnum;
	uint16_t shentsize;
	uint16_t shnum;
	uint16_t shstrndx;
};

#define ELF_HEADER_TYPE_NONE         0
#define ELF_HEADER_TYPE_OBJECT       1
#define ELF_HEADER_TYPE_EXECUTABLE   2
#define ELF_HEADER_TYPE_DYNAMIC      3
#define ELF_HEADER_TYPE_CORE         4

#define ELF_HEADER_MACHINE_I386   3
#define ELF_HEADER_MACHINE_ARM    40
#define ELF_HEADER_MACHINE_X86_64 62

#define ELF_HEADER_VERSION     1

struct elf_program {
	uint32_t type;
	uint32_t offset;
	uint32_t vaddr;
	uint32_t paddr;
	uint32_t file_size;
	uint32_t memory_size;
	uint32_t flags;
	uint32_t align;
};

#define ELF_PROGRAM_TYPE_LOADABLE 1

struct elf_section {
	uint32_t name;
	uint32_t type;
	uint32_t flags;
	uint32_t address;
	uint32_t offset;
	uint32_t size;
	uint32_t link;
	uint32_t info;
	uint32_t alignment;
	uint32_t entry_size;
};

#define ELF_SECTION_TYPE_NULL         0
#define ELF_SECTION_TYPE_PROGRAM      1
#define ELF_SECTION_TYPE_SYMBOL_TABLE 2
#define ELF_SECTION_TYPE_STRING_TABLE 3
#define ELF_SECTION_TYPE_RELA         4
#define ELF_SECTION_TYPE_HASH         5
#define ELF_SECTION_TYPE_DYNAMIC      6
#define ELF_SECTION_TYPE_NOTE         7
#define ELF_SECTION_TYPE_BSS          8

#define ELF_SECTION_FLAGS_WRITE    1
#define ELF_SECTION_FLAGS_MEMORY   2
#define ELF_SECTION_FLAGS_EXEC     8
#define ELF_SECTION_FLAGS_MERGE    16
#define ELF_SECTION_FLAGS_STRINGS  32
#define ELF_SECTION_FLAGS_INFO_LINK 64
#define ELF_SECTION_FLAGS_LINK_ORDER 128
#define ELF_SECTION_FLAGS_NONSTANDARD 256
#define ELF_SECTION_FLAGS_GROUP 512
#define ELF_SECTION_FLAGS_TLS 1024


int elf_load(struct process *p, const char *filename, addr_t * entry)
{
	struct elf_header header;
	struct elf_program program;
	struct elf_section section;
	int i;
	uint32_t actual;

	struct fs_dirent *d = fs_resolve(filename);
	if(!d)
		return KERROR_NOT_FOUND;

	actual = fs_dirent_read(d, (char *) &header, sizeof(header), 0);
	if(actual != sizeof(header))
		goto noload;

	if(strncmp(header.ident, "\177ELF", 4) || header.machine != ELF_HEADER_MACHINE_I386 || header.version != ELF_HEADER_VERSION)
		goto noexec;

	actual = fs_dirent_read(d, (char *) &program, sizeof(program), header.program_offset);
	if(actual != sizeof(program))
		goto noload;

	//printf("elf: text %x bytes from offset %x at address %x length %x\n",program.file_size,program.offset,program.vaddr,program.memory_size);

	if(program.type != ELF_PROGRAM_TYPE_LOADABLE || program.vaddr < PROCESS_ENTRY_POINT || program.memory_size > 0x8000000 || program.memory_size != program.file_size)
		goto noexec;

	process_data_size_set(p, program.memory_size);

	actual = fs_dirent_read(d, (char *) program.vaddr, program.memory_size, program.offset);
	if(actual != program.memory_size)
		goto mustdie;

	for(i = 0; i < header.shnum; i++) {
		actual = fs_dirent_read(d, (char *) &section, sizeof(section), header.section_offset + i * header.shentsize);
		if(actual != sizeof(section))
			goto mustdie;

		if(section.type == ELF_SECTION_TYPE_BSS) {
			uint32_t limit = section.address + section.size - PROCESS_ENTRY_POINT;
			if(limit > p->vm_data_size) {
				process_data_size_set(p, section.address + section.size - PROCESS_ENTRY_POINT);
				memset((void *) section.address, section.size, 0);
			}
		} else {
			/* skip all other section types */
		}
	}

	*entry = header.entry;
	fs_dirent_close(d);

	return 0;

      noload:
	printf("elf: %s failed to load correctly!\n", filename);
	fs_dirent_close(d);
	return KERROR_NOT_FOUND;

      noexec:
	printf("elf: %s is not a valid i386 ELF executable\n", filename);
	fs_dirent_close(d);
	return KERROR_NOT_EXECUTABLE;

      mustdie:
	printf("elf: %s did not load correctly\n", filename);
	fs_dirent_close(d);
	return KERROR_EXECUTION_FAILED;
}
