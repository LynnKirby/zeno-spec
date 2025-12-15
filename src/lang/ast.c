#include "src/lang/ast.h"

#include <string.h>

void AstContext_init(AstContext* context) {
    Arena_init(&context->arena);
    StringSet_init(&context->strings, &context->arena);
}

void AstContext_destroy(AstContext* context) {
    StringSet_destroy(&context->strings);
    Arena_destroy(&context->arena);
}

StringSetItem AstContext_add_string(AstContext* context, StringRef value) {
    return StringSet_add(&context->strings, value);
}
