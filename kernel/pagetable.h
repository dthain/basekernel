/*
Copyright (C) 2015-2019 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/

#ifndef PAGETABLE_H
#define PAGETABLE_H

#define PAGE_SIZE 4096

#define PAGE_FLAG_USER        0
#define PAGE_FLAG_KERNEL      1
#define PAGE_FLAG_EXISTS      0
#define PAGE_FLAG_ALLOC       2
#define PAGE_FLAG_READONLY    0
#define PAGE_FLAG_READWRITE   4
#define PAGE_FLAG_NOCLEAR     0
#define PAGE_FLAG_CLEAR       8

struct pagetable *pagetable_create();
void pagetable_init(struct pagetable *p);
int pagetable_map(struct pagetable *p, unsigned vaddr, unsigned paddr, int flags);
int pagetable_getmap(struct pagetable *p, unsigned vaddr, unsigned *paddr, int *flags);
void pagetable_unmap(struct pagetable *p, unsigned vaddr);
void pagetable_alloc(struct pagetable *p, unsigned vaddr, unsigned length, int flags);
void pagetable_free(struct pagetable *p, unsigned vaddr, unsigned length);
void pagetable_delete(struct pagetable *p);
struct pagetable *pagetable_duplicate(struct pagetable *p);
struct pagetable *pagetable_load(struct pagetable *p);
void pagetable_enable();
void pagetable_refresh();

#endif
