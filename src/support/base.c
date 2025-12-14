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
