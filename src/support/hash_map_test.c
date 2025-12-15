#undef NDEBUG

#include "src/support/hash_map.h"
#include "src/support/fnv1a.h"

#include <assert.h>

/* These types are chosen so that Value has a greater alignment than Key. */
typedef uint16_t Key;
typedef uint32_t Value;

#define KEY_MAX 60000
#define KEY_INCREMENT 42

static uint32_t terrible_hash(void const* key) {
    (void)key; /* unused */
    return 0;
}

static uint32_t key_hash(void const* key) {
    return fnv1a_add_16(fnv1a_start(), *(Key const*)key);
}

static int key_equal(void const* key1, void const* key2) {
    return *(Key const*)key1 == *(Key const*)key2;
}

static void check_entry(
    HashMap* map,
    HashMapConfig const* config,
    int is_map,
    uint32_t id,
    Key key,
    Value value
) {
    /* get_id_by_key */
    {
        uint32_t actual_id;
        actual_id = HashMap_get_id_by_key(map, config, &key);
        assert(actual_id == id);
    }

    /* get_entry_by_id */
    {
        void const* p_key;
        void const* p_value;
        HashMap_get_entry_by_id(map, config, id, &p_key, &p_value);
        assert(p_key != NULL);
        assert(*(Key const*)p_key == key);
        if (is_map) {
            assert(p_value != NULL);
            assert(*(Value const*)p_value == value);
        } else {
            assert(p_value == NULL);
        }
    }

    /* get_key_by_id */
    {
        Key const* p_key;
        p_key = HashMap_get_key_by_id(map, config, id);
        assert(p_key != NULL);
        assert(*p_key == key);
    }

    /* get_value_by_id */
    {
        Value const* p_value;
        p_value = HashMap_get_value_by_id(map, config, id);
        if (is_map) {
            assert(p_value != NULL);
            assert(*p_value == value);
        } else {
            assert(p_value == NULL);
        }
    }

    /* get_value_by_key */
    {
        Value const* p_value;
        Value* p_value_mut;
        p_value = HashMap_get_value_by_key(map, config, &key);
        p_value_mut = HashMap_get_value_by_key_mut(map, config, &key);
        assert(p_value_mut == p_value);
        if (is_map) {
            assert(p_value != NULL);
            assert(*p_value == value);
        } else {
            assert(p_value == NULL);
        }
    }
}

static void tests(HashMapConfig const* config, int is_map) {
    HashMap map;
    Key key;
    Key prev_key;
    Value prev_value = 0;
    uint32_t id = 1;

    HashMap_init(&map, config);

    /* We are empty. */
    for (key = 0; key < KEY_MAX; key += 1) {
        assert(HashMap_get_id_by_key(&map, config, &key) == 0);
    }

    for (key = 0; key < KEY_MAX; key += KEY_INCREMENT) {
        Key next_key;
        Value value;

        value = (prev_value + 1) * 2;
        next_key = key + KEY_INCREMENT;

        /* Add new entry. */
        {
            uint32_t actual_id;
            if (is_map) {
                actual_id = HashMap_set(&map, config, &key, &value);
            } else {
                actual_id = HashMap_set(&map, config, &key, NULL);
            }
            assert(actual_id == id);
        }

        /* Check curent entry. */
        check_entry(&map, config, is_map, id, key, value);

        /* Check previous entry. */
        if (key != 0) {
            check_entry(&map, config, is_map, id - 1, prev_key, prev_value);
        }

        /* Change value of previous entry and check it. */
        if (key != 0 && is_map) {
            uint32_t actual_id;
            prev_value = prev_value + 100;
            actual_id = HashMap_set(&map, config, &prev_key, &prev_value);
            assert(actual_id == id - 1);
            check_entry(&map, config, is_map, id - 1, prev_key, prev_value);
        }

        /* Next ID should not exist yet. */
        {
            uint32_t actual_id;
            actual_id = HashMap_get_id_by_key(&map, config, &next_key);
            assert(actual_id == 0);
        }

        /* Next value should not exist yet. */
        {
            uint64_t const* p_value;
            p_value = HashMap_get_value_by_key(&map, config, &next_key);
            assert(p_value == NULL);
        }

        prev_key = key;
        prev_value = value;
        id += 1;
    }

    HashMap_destroy(&map);
}

int main(void) {
    static HashMapConfig const map_config =
        HASH_MAP_CONFIG(Key, Value, key_hash, key_equal);

    static HashMapConfig const terrible_map_config =
        HASH_MAP_CONFIG(Key, Value, terrible_hash, key_equal);

    static HashMapConfig const set_config =
        HASH_SET_CONFIG(Key, key_hash, key_equal);

    static HashMapConfig const terrible_set_config =
        HASH_SET_CONFIG(Key, terrible_hash, key_equal);

    tests(&map_config, true);
    tests(&terrible_map_config, true);

    tests(&set_config, false);
    tests(&terrible_set_config, false);

    return 0;
}
