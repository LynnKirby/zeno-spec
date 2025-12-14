#ifndef _ZENO_SPEC_SRC_SUPPORT_MALLOC_H
#define _ZENO_SPEC_SRC_SUPPORT_MALLOC_H

#include <stddef.h>

#define xfree(p) xreallocarray((p), 0, 0)
#define xmalloc(n) xreallocarray(NULL, (n), 1)
#define xrealloc(p, n) xreallocarray((p), (n), 1)
#define xallocarray(n, m) xreallocarray(NULL, (n), (m))
void* xreallocarray(void* p, size_t n, size_t m);

#endif
