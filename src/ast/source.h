#ifndef _ZENO_SPEC_SRC_AST_SOURCE_H_
#define _ZENO_SPEC_SRC_AST_SOURCE_H_

#include "src/ast/string.h"
#include "src/support/stdint.h"

/** Source file owned by an AstContext.
 * `data` is nul-terminated (`data[size] == 0`). */
typedef struct SourceFile {
    AstString path;
    uint8_t const* data;
    size_t size;
} SourceFile;

#endif
