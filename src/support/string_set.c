#include "src/support/string_set.h"
#include "src/support/fnv1a.h"

#include <string.h>

static HashMapConfig set_config = HASH_SET_CONFIG(
    StringSetItem, StringSetItem_hash_generic, StringSetItem_equal_generic
);

void StringSet_init(StringSet* strings, Arena* arena) {
    HashMap_init(&strings->set, &set_config);
    strings->arena = arena;
}

void StringSet_destroy(StringSet* strings) {
    HashMap_destroy(&strings->set);
}

StringSetItem StringSet_add(StringSet* strings, StringRef value) {
    StringSetItem item;
    uint32_t id;

    item.string = value;
    item.hash = StringRef_hash(value);

    /* TODO: add HashMap API that avoids this double lookup */

    id = HashMap_get_id_by_key(&strings->set, &set_config, &item);

    if (id == 0) {
        uint8_t* copy;
        copy = Arena_allocate(strings->arena, value.size);
        memcpy(copy, value.data, value.size);
        item.string.data = copy;
        item.string.size = value.size;
        HashMap_set(&strings->set, &set_config, &item, NULL);
        return item;
    } else {
        StringSetItem const* key;
        key = HashMap_get_key_by_id(&strings->set, &set_config, id);
        return *key;
    }
}

int StringSetItem_equal(StringSetItem const* left, StringSetItem const* right) {
    return StringRef_equal(left->string, right->string);
}

uint32_t StringSetItem_hash(StringSetItem const* item) {
    return item->hash;
}

int StringSetItem_equal_generic(void const* left, void const* right) {
    return StringSetItem_equal(left, right);
}

uint32_t StringSetItem_hash_generic(void const* item) {
    return StringSetItem_hash(item);
}
