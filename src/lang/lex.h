#ifndef _ZENO_SPEC_SRC_LEX_H
#define _ZENO_SPEC_SRC_LEX_H

#include "src/lang/token.h"
#include "src/support/base.h"
#include "src/support/stdint.h"
#include "src/support/string_ref.h"

#include <setjmp.h>

typedef struct LexResult {
    int is_token;
    union {
        LexError error;
        Token token;
    } u;
} LexResult;

typedef struct LexerConfig {
    uint32_t tab_stop;
} LexerConfig;

typedef struct Lexer {
    LexerConfig config;
    uint8_t const* cursor;
    uint8_t const* limit;
    SourcePos cursor_pos;
    uint32_t characters_in_line;
    uint32_t total_characters;
    jmp_buf exit_jmp_buf;
    int done;
} Lexer;

/** Initialize lexer. `source` must include a trailing nul character.
 *  NULL config means use defaults. */
void Lexer_init(
    Lexer* lexer,
    ByteStringRef source,
    LexerConfig const* config
);

void Lexer_destroy(Lexer* lexer);

int Lexer_next(Lexer* lexer, LexResult* result);

#endif
