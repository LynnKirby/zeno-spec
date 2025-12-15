#ifndef _ZENO_SPEC_SRC_AST_SOURCE_H_
#define _ZENO_SPEC_SRC_AST_SOURCE_H_

#include "src/ast/string.h"
#include "src/support/stdint.h"

/** Source file owned by an AstContext.
 * `data` is nul-terminated (`data[size] == 0`). */
typedef struct SourceFile SourceFile;

uint8_t const* SourceFile_data(SourceFile const* source);

size_t SourceFile_size(SourceFile const* source);

AstString SourceFile_path(SourceFile const* source);

#endif
