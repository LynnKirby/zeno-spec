#ifndef _ZENO_SPEC_SRC_AST_CONTEXT_H
#define _ZENO_SPEC_SRC_AST_CONTEXT_H

#include "src/ast/source.h"
#include "src/ast/string.h"
#include "src/ast/nodes.h"
#include "src/support/io.h"

typedef struct AstContext AstContext;

/** Create AstContext instance. */
AstContext* AstContext_new(void);

/** Deinitialize and deallocate AstContext. */
void AstContext_delete(AstContext* ast);

/** Read a file and add it to the context. */
SystemIoError AstContext_source_from_file(
    AstContext* ast,
    StringRef path,
    SystemFile file,
    SourceFile const** out_source
);

/** Create a source file from a byte buffer. */
SourceFile const* AstContext_source_from_bytes(
    AstContext* ast,
    StringRef path,
    void const* data,
    size_t size
);

/** Create an interned string. */
AstString AstContext_add_string(AstContext* ast, StringRef value);

/** Allocate data owned by the context. */
void* AstContext_allocate(AstContext* ast, size_t size);

/** Get a cached SimpleType instance. */
SimpleType* AstContext_simple_type(AstContext* ast, SimpleTypeKind kind);

/** Get a cached SimpleTypeExpr instance. */
SimpleTypeExpr* AstContext_simple_type_expr(AstContext* ast, SimpleTypeKind kind);

#endif
