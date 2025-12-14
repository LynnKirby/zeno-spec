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
        Writer_print(Writer_stdout, "zeno-spec: error: out of memory\n");
        exit(1);
    }

    return p;
}
