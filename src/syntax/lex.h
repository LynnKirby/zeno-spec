#ifndef _ZENO_SPEC_SRC_SYNTAX_LEX_H
#define _ZENO_SPEC_SRC_SYNTAX_LEX_H

#include "src/ast/context.h"
#include "src/ast/source.h"
#include "src/support/stdint.h"
#include "src/syntax/token.h"

#include <setjmp.h>

typedef struct LexerConfig {
    uint32_t tab_stop;
} LexerConfig;

typedef struct LexResult {
    int is_tokens;
    union {
        TokenList tokens;
        LexError error;
    } u;
} LexResult;

void lex_source(
    LexResult* result,
    AstContext* ast,
    SourceFile const* source,
    LexerConfig const* config
);

#endif
