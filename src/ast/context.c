#include "src/ast/context.h"
#include "src/ast/source_internal.h"
#include "src/support/arena.h"
#include "src/support/hash_map.h"
#include "src/support/malloc.h"

#include <assert.h>
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

    /* Cached types */
    SimpleType simple_types[SimpleTypeKind_COUNT];
    SimpleTypeExpr simple_type_exprs[SimpleTypeKind_COUNT];
};

AstContext* AstContext_new(void) {
    AstContext* ast;

    ast = xmalloc(sizeof(AstContext));

    Arena_init(&ast->arena);
    HashMap_init(&ast->string_set, &string_set_config);
    ast->files_data  = NULL;
    ast->files_size = 0;
    ast->files_capacity = 0;

    /* Construct simple types. */
    {
        int i;
        for (i = 0; i < SimpleTypeKind_COUNT; i += 1) {
            ast->simple_types[i].base.kind = TypeKind_Simple;
            ast->simple_types[i].kind = i;

            ast->simple_type_exprs[i].base.kind = ExprKind_SimpleType;
            ast->simple_type_exprs[i].kind = i;
        }
    }

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

static SourceFile const* add_file(
    AstContext* ast, StringRef path, void const* data, size_t size
) {
    SourceFile* source;

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

    return source;
}

SystemIoError AstContext_source_from_file(
    AstContext* ast,
    StringRef path,
    SystemFile file,
    SourceFile const** out_source
) {
    SystemIoError res;
    void* data;
    size_t size;

    res = SystemFile_read_all(file, &data, &size);

    if (res == SystemIoError_Success) {
        *out_source = add_file(ast, path, data, size);
    }

    return res;
}

SourceFile const* AstContext_source_from_bytes(
    AstContext* ast,
    StringRef path,
    void const* data,
    size_t size
) {
    char* copy;

    copy = xmalloc(size + 1);
    memcpy(copy, data, size);
    copy[size] = 0;

    return add_file(ast, path, copy, size);
}

SimpleType* AstContext_simple_type(AstContext* ast, SimpleTypeKind kind) {
    assert(kind >= 0);
    assert(kind < SimpleTypeKind_COUNT);
    return &ast->simple_types[kind];
}

SimpleTypeExpr* AstContext_simple_type_expr(
    AstContext* ast, SimpleTypeKind kind
) {
    assert(kind >= 0);
    assert(kind < SimpleTypeKind_COUNT);
    return &ast->simple_type_exprs[kind];
}
