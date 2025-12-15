#ifndef _ZENO_SPEC_SRC_LEX_H
#define _ZENO_SPEC_SRC_LEX_H

#include "src/lang/token.h"
#include "src/support/stdint.h"
#include "src/support/string_ref.h"

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

void lex_bytes(
    LexResult* result, ByteStringRef source, LexerConfig const* config
);

#endif
