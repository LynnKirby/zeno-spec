#ifndef _ZENO_SPEC_SRC_SUPPORT_STRING_REF_H
#define _ZENO_SPEC_SRC_SUPPORT_STRING_REF_H

#include <stddef.h>

/** Borrowed reference to a byte string (do not assume the encoding). */
typedef struct ByteStringRef {
    char const* data;
    size_t size;
} ByteStringRef;

/** Borrowed reference to a UTF-8 string. */
typedef struct StringRef {
    uint8_t const* data;
    size_t size;
} StringRef;

/** Static initializer for a StringRef using a string literal. */
#define STATIC_STRING_REF(s) { (uint8_t const*)(s), sizeof(s) - 1 }

#endif
