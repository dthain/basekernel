/* malloc.c: simple memory allocator -----------------------------------------*/
#include "syscalls.h"


/* Macros --------------------------------------------------------------------*/

#define ALIGN4(s)           (((((s) - 1) >> 2) << 2) + 4)
#define BLOCK_DATA(b)       ((b) + 1)
#define BLOCK_HEADER(ptr)   ((struct block *)(ptr) - 1)

/* Block structure -----------------------------------------------------------*/

struct block {
    uint32_t        size;
    struct block *next;
    bool          free;
};

/* Global variables ----------------------------------------------------------*/

struct block *FreeList = 0;

/* TODO: Add counters for mallocs, frees, reuses, grows */

/* Find free block -----------------------------------------------------------*/

struct block *find_free(struct block **last, uint32_t size) {
    struct block *curr = FreeList;

    /* First fit */
    while (curr && !(curr->free && curr->size >= size)) {
        *last = curr;
        curr  = curr->next;
    }

    return curr;
}

/* Grow heap -----------------------------------------------------------------*/

struct block *grow_heap(struct block *last, uint32_t size) {
    /* Request more space from OS */
    struct block *curr = (struct block *)sbrk(0);
    struct block *prev = (struct block *)sbrk(sizeof(struct block) + size);

    /* OS allocation failed */
    if (curr == (struct block *)-1) {
        return 0;
    }

    /* Update FreeList if not set */
    if (!FreeList) {
        FreeList = curr;
    }

    /* Attach new block to prev block */
    if (last) {
        last->next = curr;
    }

    /* Update block metadata */
    curr->size = size;
    curr->next = 0;
    curr->free = 0;
    return curr;
}

/* Allocate space ------------------------------------------------------------*/

void *malloc(uint32_t size) {
    /* Align to multiple of 4 */
    size = ALIGN4(size);

    /* Handle 0 size */
    if (size == 0) {
        return 0;
    }

    /* Look for free block */
    struct block *last = FreeList;
    struct block *next = find_free(&last, size);

    /* TODO: Split free block? */

    /* Could not find free block, so grow heap */
    if (!next) {
        next = grow_heap(last, size);
    }

    /* Could not find free block or grow heap, so just return 0 */
    if (!next) {
        return 0;
    }
    
    /* Mark block as in use */
    next->free = 0;

    /* Return data address associated with block */
    return BLOCK_DATA(next);
}

/* Reclaim space -------------------------------------------------------------*/

void free(void *ptr) {
    if (!ptr) {
        return;
    }

    /* Make block as free */
    struct block *curr = BLOCK_HEADER(ptr);
    curr->free = 1;

    /* TODO: Coalesce free blocks? */
}

/* vim: set expandtab sts=4 sw=4 ts=8 ft=cpp: --------------------------------*/
