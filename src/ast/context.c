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

    /* Set[AstString] */
    HashMap string_set;

    /** ArrayList[SourceFile*] */
    SourceFile const** files_data;
    size_t files_size;
    size_t files_capacity;
};

AstContext* AstContext_new(void) {
    AstContext* ast;

    ast = xmalloc(sizeof(AstContext));

    Arena_init(&ast->arena);
    HashMap_init(&ast->string_set, &string_set_config);
    ast->files_data  = NULL;
    ast->files_size = 0;
    ast->files_capacity = 0;

    return ast;
}

void AstContext_delete(AstContext* ast) {
    size_t i;
    for (i = 0; i < ast->files_size; i += 1) {
        xfree((void*)ast->files_data[i]->data);
    }
    xfree(ast->files_data);
    HashMap_destroy(&ast->string_set);
    Arena_destroy(&ast->arena);
    xfree(ast);
}

AstString AstContext_add_string(AstContext* ast, StringRef value) {
    AstString item;
    uint32_t id;

    AstString_init(&item, value);

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

SystemIoError AstContext_add_file(
    AstContext* ast,
    StringRef path,
    SystemFile file,
    SourceFile const** out_source
) {
    SourceFile* source;
    SystemIoError res;
    void* data;
    size_t size;

    res = SystemFile_read_all(file, &data, &size);

    if (res != SystemIoError_Success) {
        return res;
    }

    source = AstContext_allocate(ast, sizeof(SourceFile));
    source->path = AstContext_add_string(ast, path);
    source->data = data;
    source->size = size;

    ast->files_data = ensure_array_capacity(
        sizeof(SourceFile*),
        ast->files_data,
        &ast->files_size,
        &ast->files_capacity,
        1
    );
    ast->files_data[ast->files_size] = source;
    ast->files_size += 1;

    *out_source = source;
    return SystemIoError_Success;
}
