#ifndef _ZENO_SPEC_SRC_AST_CONTEXT_H
#define _ZENO_SPEC_SRC_AST_CONTEXT_H

#include "src/support/string_ref.h"

#include <stddef.h>

typedef struct AstContext AstContext;

typedef struct AstString {
    StringRef value;
    uint32_t hash;
} AstString;

/** Create AstContext instance. */
AstContext* AstContext_new(void);

/** Deinitialize and deallocate AstContext. */
void AstContext_delete(AstContext* ast);

/** Create an interned string. */
AstString AstContext_add_string(AstContext* ast, StringRef value);

/** Allocate data owned by the context. */
void* AstContext_allocate(AstContext* ast, size_t size);

int AstString_equal(AstString const* left, AstString const* right);
uint32_t AstString_hash(AstString const* item);

int AstString_equal_generic(void const* left, void const* right);
uint32_t AstString_hash_generic(void const* item);

#endif
