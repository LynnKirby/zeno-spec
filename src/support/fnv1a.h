#ifndef _ZENO_SPEC_SUPPORT_FNV1A_H
#define _ZENO_SPEC_SUPPORT_FNV1A_H

#include "src/support/defs.h"
#include "src/support/stdint.h"

static inline uint32_t fnv1a_start(void) {
    return 2166136261u;
}

static inline uint32_t fnv1a_add(
    uint32_t hash, void const* bytes, size_t size
) {
    size_t i;
    for (i = 0; i < size; i += 1) {
        hash = (hash ^ ((uint8_t const*)bytes)[i]) * 16777619u;
    }
    return hash;
}

static inline uint32_t fnv1a_add_8(uint32_t hash, uint8_t value) {
    return fnv1a_add(hash, &value, 1);
}

static inline uint32_t fnv1a_add_16(uint32_t hash, uint16_t value) {
    return fnv1a_add(hash, &value, 2);
}

static inline uint32_t fnv1a_add_32(uint32_t hash, uint32_t value) {
    return fnv1a_add(hash, &value, 4);
}

#endif
