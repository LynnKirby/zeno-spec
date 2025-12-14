#ifndef _ZENO_SPEC_SRC_SUPPORT_ARENA_H
#define _ZENO_SPEC_SRC_SUPPORT_ARENA_H

#include <stddef.h>

/** Arena allocator. */
typedef struct Arena {
    struct ArenaChunk* chunks;
} Arena;

void Arena_init(Arena* arena);
void Arena_destroy(Arena* arena);
void* Arena_allocate(Arena* arena, size_t size);


#endif
