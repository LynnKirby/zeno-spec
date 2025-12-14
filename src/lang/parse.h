#ifndef _ZENO_SPEC_SRC_PARSE_H
#define _ZENO_SPEC_SRC_PARSE_H

#include "src/lang/ast.h"
#include "src/lang/lex.h"
#include "src/support/string_ref.h"

typedef enum ParseResultKind {
    ParseResultKind_Success,
    ParseResultKind_LexError,
    ParseResultKind_ParseError,
    ParseResultKind_YaccError
} ParseResultKind;

typedef struct ParseError {
    SourcePos token_pos;
    TokenKind token_kind;
} ParseError;

typedef struct ParseResult {
    ParseResultKind kind;
    union {
        Item* syntax;
        ParseError parse_error;
        LexError lex_error;
        ByteStringRef yacc_error;
    } u;
} ParseResult;

void parse(ParseResult* result, AstContext* context, Lexer* lexer);

#endif
