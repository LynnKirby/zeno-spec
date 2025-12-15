#ifndef _ZENOC_SRC_SUPPORT_STRING_SET_H
#define _ZENOC_SRC_SUPPORT_STRING_SET_H

#include "src/support/arena.h"
#include "src/support/hash_map.h"
#include "src/support/stdint.h"
#include "src/support/string_ref.h"

typedef struct StringSetItem {
    StringRef string;
    uint32_t hash;
} StringSetItem;

typedef struct StringSet {
    HashMap set;
    Arena* arena;
} StringSet;

void StringSet_init(StringSet* strings, Arena* arena);
void StringSet_destroy(StringSet* strings);
StringSetItem StringSet_add(StringSet* strings, StringRef value);

int StringSetItem_equal(StringSetItem const* left, StringSetItem const* right);
uint32_t StringSetItem_hash(StringSetItem const* item);

int StringSetItem_equal_generic(void const* left, void const* right);
uint32_t StringSetItem_hash_generic(void const* item);

#endif
