#include "src/support/malloc.h"
#include "src/support/io.h"

#include <stdlib.h>

void* xreallocarray(void* p, size_t n, size_t m) {
    size_t total;

    total = n * m; /* FIXME: overflow */

    if (total == 0) {
        free(p);
        return NULL;
    }

    p = realloc(p, total);

    if (p == NULL) {
        Writer_format(Writer_stdout, "zeno-spec: error: out of memory\n");
        exit(1);
    }

    return p;
}

void* ensure_array_capacity(
    size_t item_size,
    void* data,
    size_t* size,
    size_t* capacity,
    size_t extra_capacity
) {
    if ((*capacity - *size) >= extra_capacity) {
        return data;
    }

    if (*capacity == 0) {
        *capacity = 16;
    }

    while (*capacity - *size < extra_capacity) {
        /* FIXME: check overflow */
        *capacity += *capacity / 2;
    }

    data = xreallocarray(data, *capacity, item_size);
    return data;
}
