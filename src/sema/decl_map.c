#include "src/sema/decl_map.h"
#include "src/support/malloc.h"

#include <assert.h>

static HashMapConfig const map_config = HASH_MAP_CONFIG(
    AstString,
    Decl,
    AstString_hash_generic,
    AstString_equal_generic
);

void DeclMap_init(DeclMap* decls) {
    decls->active = NULL;
    decls->cache = NULL;
    DeclMap_push_scope(decls);
}

static void destroy_node_stack(DeclMapNode* node) {
    while (node != NULL) {
        DeclMapNode* next;
        next = node->next;
        HashMap_destroy(&node->map);
        xfree(node);
        node = next;
    }
}

void DeclMap_destroy(DeclMap* decls) {
    destroy_node_stack(decls->active);
    destroy_node_stack(decls->cache);
}

void DeclMap_push_scope(DeclMap* decls) {
    DeclMapNode* new_top;

    if (decls->cache != NULL) {
        /* Pop cached if possible */
        new_top = decls->cache;
        decls->cache = new_top->next;
    } else {
        /* Otherwise allocate new */
        new_top = xmalloc(sizeof(DeclMapNode));
        HashMap_init(&new_top->map, &map_config);
    }

    new_top->next = decls->active;
    decls->active = new_top;
}

void DeclMap_pop_scope(DeclMap* decls) {
    DeclMapNode* old_top;
    assert(decls->active != NULL);

    /* Pop active */
    old_top = decls->active;
    decls->active = old_top->next;

    /* Push cached */
    old_top->next = decls->cache;
    decls->cache = old_top;

    HashMap_reset(&old_top->map);
}

Decl const* DeclMap_get(DeclMap* decls, AstString name) {
    DeclMapNode* node;
    node = decls->active;

    while (node != NULL) {
        Decl const* value;
        value = HashMap_get_value_by_key(&node->map, &map_config, &name);
        if (value != NULL) {
            return value;
        }
        node = node->next;
    }

    return NULL;
}

void DeclMap_set(DeclMap* decls, AstString name, Decl const* decl) {
    HashMap_set(&decls->active->map, &map_config, &name, decl);
}
