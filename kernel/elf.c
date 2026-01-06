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

#define ELF_PROGRAM_TYPE_NULL 0
#define ELF_PROGRAM_TYPE_LOADABLE 1
#define ELF_PROGRAM_TYPE_DYNAMIC 2
#define ELF_PROGRAM_TYPE_INTERPRETER 3
#define ELF_PROGRAM_TYPE_NOTE 4
#define ELF_PROGRAM_TYPE_SHARED_LIBRARY 5
#define ELF_PROGRAM_TYPE_PROGRAM_HEADER 6
#define ELF_PROGRAM_TYPE_THREAD_LOCAL 7
#define ELF_PROGRAM_TYPE_GNU_EH_FRAME 0x6474e550
#define ELF_PROGRAM_TYPE_GNU_STACK 0x6474e551
#define ELF_PROGRAM_TYPE_GNU_RELRO 0x6474e552

#define ELF_PROGRAM_FLAGS_EXEC  1
#define ELF_PROGRAM_FLAGS_WRITE 2
#define ELF_PROGRAM_FLAGS_READ  4

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

/* Ensure that the current process has address space up to this value. */

static int elf_ensure_address_space( struct process *p, uint32_t addr )
{
	/* Size of user data area, ignoring start addr */
	uint32_t limit = addr - PROCESS_ENTRY_POINT;

	/* Round up to next page size. */
	uint32_t overflow = limit % PAGE_SIZE;
	limit += (PAGE_SIZE-overflow);

	/* Extend virtual memory if needed. */
	if(limit > p->vm_data_size) {
		return process_data_size_set(p,limit);
	} else {
		return 0;
	}

	/* Return zero on success. */
}

/* Load an ELF executable into user space. */

int elf_load(struct process *p, struct fs_dirent *d, addr_t * entry)
{
	struct elf_header header;
	struct elf_program program;
	int i;
	uint32_t actual;

	/* Load the overall ELF header from the beginning of the file. */
	actual = fs_dirent_read(d, (char *) &header, sizeof(header), 0);
	if(actual != sizeof(header)) {
		printf("elf: unable to load elf header.\n");
		return KERROR_NOT_EXECUTABLE;
	}

	/* Bail out if the header doesnt not have the expected values. */
	if(strncmp(header.ident, "\177ELF", 4) || header.machine != ELF_HEADER_MACHINE_I386 || header.version != ELF_HEADER_VERSION) {
		printf("elf: not a valid i386 executable file.\n");
		return KERROR_NOT_EXECUTABLE;
	}

	/* An elf file contains a sequence of "program headers" that correspond to loadable segments. */
	for(i = 0; i < header.phnum; i++) {

		/* Load in the segment header itself. */
		actual = fs_dirent_read(d, (char *) &program, sizeof(program), header.program_offset + i * header.phentsize);
		if(actual != sizeof(program)) {
			printf("elf: unable to load segment header %d.\n",i);
			return KERROR_NOT_EXECUTABLE;
		}

		/* Safe to skip segments that are not loadable or zero size: */
		if(program.type != ELF_PROGRAM_TYPE_LOADABLE || program.memory_size==0 ) {
			continue;
		}

		/* Each segment must be within the expected userspace range. */
		if(program.vaddr < PROCESS_ENTRY_POINT || program.memory_size > 0x8000000) {
			printf("elf: segment %d is invalid: vaddr %x size %x lies outside of user address space.\n",i,program.vaddr,program.memory_size);
			return KERROR_NOT_EXECUTABLE;
		}

		/* Check for unexpected segment configuration. */
		if(program.file_size > program.memory_size) {
			printf("elf: segment %d has unexpected file size %x smaller than memory size %x.\n",program.file_size,program.memory_size);
			return KERROR_NOT_EXECUTABLE;
		}


		/* Expand the user address space if needed for this segment. */
		if(elf_ensure_address_space(p,program.vaddr + program.memory_size)!=0) {
			printf("elf: unable to allocate memory for segment %d vaddr %x size %x\n",i,program.vaddr,program.memory_size);
			return KERROR_OUT_OF_MEMORY;
		}

		/* If some (or all) of this segment is on disk, load it in. */
		if(program.file_size>0) {
			actual = fs_dirent_read(d, (char *) program.vaddr, program.file_size, program.offset);
			if(actual != program.file_size) {
				printf("elf: unable to load segment %d from disk.\n");
				return KERROR_NOT_EXECUTABLE;
			}
		}

		/* If the remainder (or all) of this segment is BSS, initialize it. */
		if(program.memory_size>program.file_size) {
			memset( (char*) (program.vaddr+program.file_size), program.memory_size-program.file_size, 0 );
		}
		
		/* XXX Set page table bits here. */
	}

	/* Capture the program entry point for the caller to use. */
	*entry = header.entry;
	return 0;
}
