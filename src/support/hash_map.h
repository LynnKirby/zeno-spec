#ifndef _ZENOC_SRC_SUPPORT_HASH_MAP
#define _ZENOC_SRC_SUPPORT_HASH_MAP

#include "src/support/stdint.h"

#include <stddef.h>

/*
 * Features of our hash map:
 * - Maintains insertion order.
 * - Append-only, no removals.
 * - All entries have a unique 32-bit ID and can be looked up by it.
 * - Zero ID is reserved and invalid.
 * - `indexes` is a compact hash table that uses uint{8,16,32}_t indexes
 *   based on the size of the table.
 * - `entries` is a densely packed array of key-value entries, indexed by ID.
 */

typedef struct HashMapConfig {
    uint32_t (*hash)(void const* key);
    int (*equal)(void const* key1, void const* key2);
    size_t key_size;
    size_t value_size;
    size_t value_offset;
    size_t entry_size;
} HashMapConfig;

#define HASH_MAP_CONFIG(Key, Value, hash, equal) \
    {                                            \
        (hash),                                  \
        (equal),                                 \
        sizeof(Key),                             \
        sizeof(Value),                           \
        offsetof(struct { Key k; Value v; }, v), \
        sizeof(struct { Key k; Value v; })       \
    }

#define HASH_SET_CONFIG(Type, hash, equal) \
    { (hash), (equal), sizeof(Type), 0, 0, sizeof(Type) }

typedef struct HashMap {
    char* entries;
    void* buckets;
    uint32_t entries_count;
    uint32_t entries_capacity;
    uint32_t buckets_count;
} HashMap;

void HashMap_init(HashMap* map, HashMapConfig const* config);

void HashMap_destroy(HashMap* map);

void HashMap_reset(HashMap* map);

uint32_t HashMap_set(
    HashMap* map,
    HashMapConfig const* config,
    void const* key,
    void const* value
);

uint32_t HashMap_get_id_by_key(
    HashMap const* map, HashMapConfig const* config, void const* key
);

void HashMap_get_entry_by_id(
    HashMap const* map,
    HashMapConfig const* config,
    uint32_t id,
    void const** key,
    void const** value
);

void const* HashMap_get_key_by_id(
    HashMap const* map, HashMapConfig const* config, uint32_t id
);

void const* HashMap_get_value_by_id(
    HashMap const* map, HashMapConfig const* config, uint32_t id
);

void const* HashMap_get_value_by_key(
    HashMap const* map, HashMapConfig const* config, void const* key
);

void* HashMap_get_value_by_key_mut(
    HashMap* map, HashMapConfig const* config, void const* key
);

#endif
