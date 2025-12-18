#ifndef _ZENO_SPEC_SRC_PARSING_PARSE_H
#define _ZENO_SPEC_SRC_PARSING_PARSE_H

#include "src/ast/context.h"
#include "src/ast/nodes.h"
#include "src/parsing/token.h"
#include "src/support/string_ref.h"

typedef enum ParseResultKind {
    ParseResultKind_Success,
    ParseResultKind_ParseError,
    ParseResultKind_YaccError
} ParseResultKind;

#define SYNTAX_CATEGORY_LIST(X) \
    X(Item, "item")             \
    X(Stmt, "statement")        \
    X(Expr, "expression")       \
    X(Type, "type")

typedef enum SyntaxCategory {
    #define X(name, str) SyntaxCategory_##name,
    SYNTAX_CATEGORY_LIST(X)
    #undef X
    SyntaxCategory_COUNT ATTR_UNUSED
} SyntaxCategory;

typedef struct ParseError {
    SyntaxCategory expected_category;
    SourcePos actual_token_pos;
    TokenKind actual_token_kind;
} ParseError;

typedef struct ParseResult {
    ParseResultKind kind;
    union {
        Item* item;
        ParseError parse_error;
        ByteStringRef yacc_error;
    } u;
} ParseResult;

void parse(ParseResult* result, AstContext* context, TokenList const* tokens);

#endif
