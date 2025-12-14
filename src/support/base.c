#include "src/support/base.h"
#include "src/support/io.h"

#include <stdlib.h>

void* xmalloc(size_t size) {
    void* p;
    p = malloc(size);
    if (p == NULL) {
        Writer_print(Writer_stdout, "zeno-spec: error: out of memory\n");
        exit(1);
    }
    return p;
}

/* TODO: use actual arena allocation but keep this per-allocation chunk
 * implementation as an option. Useful for checking under AddressSanitizer. */

typedef struct ArenaChunk {
    struct ArenaChunk* next;
    void* data;
} ArenaChunk;

void Arena_init(Arena* arena) {
    arena->chunks = NULL;
}

void Arena_destroy(Arena* arena) {
    ArenaChunk* chunk;
    chunk = arena->chunks;

    while (chunk != NULL) {
        ArenaChunk* next;
        next = chunk->next;
        free(chunk->data);
        free(chunk);
        chunk = next;
    }
}

void* Arena_allocate(Arena* arena, size_t size) {
    ArenaChunk* chunk;
    void* data;

    chunk = xmalloc(sizeof(ArenaChunk));
    data = xmalloc(size);
    chunk->data = data;
    chunk->next = arena->chunks;
    arena->chunks = chunk;

    return data;
}
