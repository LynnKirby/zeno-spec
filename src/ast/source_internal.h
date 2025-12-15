#ifndef _ZENO_SPEC_SRC_AST_SOURCE_INTERNAL_H
#define _ZENO_SPEC_SRC_AST_SOURCE_INTERNAL_H

#include "src/ast/source.h"

struct SourceFile {
    AstString path;
    uint8_t const* data;
    size_t size;
};

#endif
