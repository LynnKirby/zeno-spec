#include "src/support/hash_map.h"
#include "src/support/malloc.h"

#include <assert.h>
#include <string.h>

static void* get_entry_from_id(
    HashMap const* map, HashMapConfig const* config, uint32_t id
) {
    assert(id != 0);
    assert((id - 1) < map->entries_capacity);
    return map->entries + (config->entry_size * (id - 1));
}

static uint32_t get_id_from_bucket(HashMap const* map, uint32_t bucket) {
    /* Comparisons are < MAX and not <= MAX because we exclude index 0. */
    if (map->buckets_count < UINT8_MAX) {
        return ((uint8_t const*)map->buckets)[bucket];
    } else if (map->buckets_count < UINT16_MAX) {
        return ((uint16_t const*)map->buckets)[bucket];
    } else {
        return ((uint32_t const*)map->buckets)[bucket];
    }
}

static void set_bucket_id(HashMap* map, uint32_t bucket, uint32_t id) {
    if (map->buckets_count < UINT8_MAX) {
        ((uint8_t*)map->buckets)[bucket] = id;
    } else if (map->buckets_count < UINT16_MAX) {
        ((uint16_t*)map->buckets)[bucket] = id;
    } else {
        ((uint32_t*)map->buckets)[bucket] = id;
    }
}

static uint32_t get_internal(
    HashMap const* map,
    HashMapConfig const* config,
    char const* key,
    uint32_t* out_bucket
) {
    uint32_t hash;
    uint32_t id;
    uint32_t bucket;

    hash = config->hash(key);
    bucket = hash % map->buckets_count;

    for (;;) {
        uint32_t entry_hash;
        char const* entry;

        if (bucket == map->buckets_count) {
            /* Roll over. */
            bucket = 0;
        }

        id = get_id_from_bucket(map, bucket);

        if (id == 0) {
            if (out_bucket != NULL) {
                *out_bucket = bucket;
            }
            return 0;
        }

        entry = get_entry_from_id(map, config, id);
        entry_hash = config->hash(entry);

        if (entry_hash == hash && config->equal(key, entry)) {
            return id;
        }

        bucket += 1;
    }
}

static void maybe_resize(HashMap* map, HashMapConfig const* config) {
    uint32_t id;

    /* 75% load factor */
    if ((map->buckets_count / 4) * 3 > map->entries_count) {
        return;
    }

    /* FIXME: overflow */
    map->buckets_count = map->buckets_count * 2;

    xfree(map->buckets);

    if (map->buckets_count < UINT8_MAX) {
        map->buckets = xallocarray(map->buckets_count, 1);
        memset(map->buckets, 0, map->buckets_count);
    } else if (map->buckets_count < UINT16_MAX) {
        map->buckets = xallocarray(map->buckets_count, 2);
        memset(map->buckets, 0, map->buckets_count * 2);
    } else {
        map->buckets = xallocarray(map->buckets_count, 4);
        memset(map->buckets, 0, map->buckets_count * 4);
    }

    for (id = 1; id <= map->entries_count; id += 1) {
        char* entry;
        uint32_t existing_id;
        uint32_t bucket;
        entry = get_entry_from_id(map, config, id);
        existing_id = get_internal(map, config, entry, &bucket);
        assert(existing_id == 0);
        set_bucket_id(map, bucket, id);
    }
}

void HashMap_init(HashMap* map, HashMapConfig const* config) {
    map->entries_count = 0;
    map->entries_capacity = 16;
    map->buckets_count = 16;

    map->entries = xallocarray(config->entry_size, map->entries_capacity);

    map->buckets = xmalloc(map->buckets_count);
    memset(map->buckets, 0, map->buckets_count);
}

void HashMap_destroy(HashMap* map) {
    xfree(map->entries);
    xfree(map->buckets);
}

uint32_t HashMap_set(
    HashMap* map,
    HashMapConfig const* config,
    void const* key,
    void const* value
) {
    uint32_t id;
    uint32_t bucket;
    char* entry;

    id = get_internal(map, config, key, &bucket);

    if (id != 0) {
        if (config->value_size > 0) {
            entry = get_entry_from_id(map, config, id);
            memcpy(entry + config->value_offset, value, config->value_size);
        }
        return id;
    }

    id = map->entries_count + 1;

    if (map->entries_count == map->entries_capacity) {
        /* Expand entries array. 1.5x growth rate. */
        /* FIXME: overflow */
        map->entries_capacity += map->entries_capacity / 2;
        map->entries = xreallocarray(
            map->entries, config->entry_size, map->entries_capacity
        );
    }

    /* Create entry. */
    entry = get_entry_from_id(map, config, id);
    memcpy(entry, key, config->key_size);
    if (config->value_size > 0) {
        memcpy(entry + config->value_offset, value, config->value_size);
    }
    map->entries_count += 1;

    /* Set bucket. */
    set_bucket_id(map, bucket, id);

    /* Resize if load factor exceeded. */
    maybe_resize(map, config);

    return id;
}

uint32_t HashMap_get_id_by_key(
    HashMap const* map, HashMapConfig const* config, void const* key
) {
    return get_internal(map, config, key, NULL);
}

void const* HashMap_get_value_by_key(
    HashMap const* map, HashMapConfig const* config, void const* key
) {
    uint32_t id;
    char const* entry;

    id = get_internal(map, config, key, NULL);

    if (id == 0) {
        return NULL;
    }

    if (config->value_size == 0) {
        return NULL;
    }

    entry = get_entry_from_id(map, config, id);
    return entry + config->value_offset;
}

void* HashMap_get_value_by_key_mut(
    HashMap* map, HashMapConfig const* config, void const* key
) {
    return (void*)HashMap_get_value_by_key(map, config, key);
}

void HashMap_get_entry_by_id(
    HashMap const* map,
    HashMapConfig const* config,
    uint32_t id,
    void const** key,
    void const** value
) {
    char const* entry;
    assert(id != 0);
    assert((id - 1) < map->entries_count);
    entry = get_entry_from_id(map, config, id);
    *key = entry;
    if (config->value_size == 0) {
        *value = NULL;
    } else {
        *value = entry + config->value_offset;
    }
}

void const* HashMap_get_key_by_id(
    HashMap const* map, HashMapConfig const* config, uint32_t id
) {
    void const* key;
    void const* value;
    HashMap_get_entry_by_id(map, config, id, &key, &value);
    return key;
}

void const* HashMap_get_value_by_id(
    HashMap const* map, HashMapConfig const* config, uint32_t id
) {
    void const* key;
    void const* value;
    HashMap_get_entry_by_id(map, config, id, &key, &value);
    return value;
}
