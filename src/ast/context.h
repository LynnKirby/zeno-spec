#ifndef _ZENO_SPEC_SRC_AST_CONTEXT_H
#define _ZENO_SPEC_SRC_AST_CONTEXT_H

#include "src/ast/source.h"
#include "src/ast/string.h"
#include "src/support/io.h"

typedef struct AstContext AstContext;

/** Create AstContext instance. */
AstContext* AstContext_new(void);

/** Deinitialize and deallocate AstContext. */
void AstContext_delete(AstContext* ast);

/** Read a file and add it to the context. */
SystemIoError AstContext_add_file(
    AstContext* ast,
    StringRef path,
    SystemFile file,
    SourceFile const** out_source
);

/** Create an interned string. */
AstString AstContext_add_string(AstContext* ast, StringRef value);

/** Allocate data owned by the context. */
void* AstContext_allocate(AstContext* ast, size_t size);

#endif
