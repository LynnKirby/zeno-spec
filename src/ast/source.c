#include "src/ast/source.h"
#include "src/ast/source_internal.h"

uint8_t const* SourceFile_data(SourceFile const* source) {
    return source->data;
}

size_t SourceFile_size(SourceFile const* source) {
    return source->size;
}

AstString SourceFile_path(SourceFile const* source) {
    return source->path;
}
