#include "src/ast/context.h"
#include "src/support/arena.h"
#include "src/support/hash_map.h"
#include "src/support/malloc.h"

#include <string.h>

static HashMapConfig string_set_config = HASH_SET_CONFIG(
    AstString, AstString_hash_generic, AstString_equal_generic
);

struct AstContext {
    Arena arena;
    HashMap string_set; /* Set[AstString] */
};

AstContext* AstContext_new(void) {
    AstContext* ast;

    ast = xmalloc(sizeof(AstContext));

    Arena_init(&ast->arena);
    HashMap_init(&ast->string_set, &string_set_config);

    return ast;
}

void AstContext_delete(AstContext* ast) {
    HashMap_destroy(&ast->string_set);
    Arena_destroy(&ast->arena);
    xfree(ast);
}

AstString AstContext_add_string(AstContext* ast, StringRef value) {
    AstString item;
    uint32_t id;

    item.value = value;
    item.hash = StringRef_hash(value);

    /* TODO: add HashMap API that avoids this double lookup */

    id = HashMap_get_id_by_key(&ast->string_set, &string_set_config, &item);

    if (id == 0) {
        uint8_t* copy;
        copy = AstContext_allocate(ast, value.size);
        memcpy(copy, value.data, value.size);
        item.value.data = copy;
        item.value.size = value.size;
        HashMap_set(&ast->string_set, &string_set_config, &item, NULL);
        return item;
    } else {
        AstString const* key;
        key = HashMap_get_key_by_id(&ast->string_set, &string_set_config, id);
        return *key;
    }
}

void* AstContext_allocate(AstContext* ast, size_t size) {
    return Arena_allocate(&ast->arena, size);
}

int AstString_equal(AstString const* left, AstString const* right) {
    return StringRef_equal(left->value, right->value);
}

uint32_t AstString_hash(AstString const* item) {
    return item->hash;
}

int AstString_equal_generic(void const* left, void const* right) {
    return AstString_equal(left, right);
}

uint32_t AstString_hash_generic(void const* item) {
    return AstString_hash(item);
}
