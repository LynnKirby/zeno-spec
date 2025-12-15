#include "src/syntax/lex.h"
#include "src/support/malloc.h"

int LLVMFuzzerTestOneInput(uint8_t const* data, size_t size) {
    AstContext* ast;
    SourceFile const* source;
    StringRef name = STATIC_STRING_REF("fuzz.zn");
    LexResult lex_result;

    ast = AstContext_new();

    source = AstContext_source_from_bytes(ast, name, data, size);

    lex_source(&lex_result, ast, source, NULL);

    if (lex_result.is_tokens) {
        xfree(lex_result.u.tokens.data);
    }

    AstContext_delete(ast);

    return 0;
}
