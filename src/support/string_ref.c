#include "src/support/string_ref.h"
#include "src/support/fnv1a.h"

#include <string.h>

int StringRef_equal(StringRef left, StringRef right) {
    return (left.size == right.size)
        && (memcmp(left.data, right.data, left.size) == 0);
}

uint32_t StringRef_hash(StringRef string) {
    return fnv1a_add(fnv1a_start(), string.data, string.size);
}

uint32_t StringRef_hash_generic(void const* key) {
    return StringRef_hash(*(StringRef const*)key);
}

int StringRef_equal_generic(void const* left, void const* right) {
    return StringRef_equal(
        *(StringRef const*)left,
        *(StringRef const*)right
    );
}
