#include "src/token.h"

#include <assert.h>

static StringRef const token_kind_names[] = {
    #define X(name, str) {(#name), sizeof(#name) - 1},
    TOKEN_KIND_LIST(X)
    #undef X
};

static StringRef const token_kind_spellings[] = {
    #define X(name, str) {(str), sizeof(str) - 1},
    TOKEN_KIND_LIST(X)
    #undef X
};

static StringRef const lex_error_kind_names[] = {
    #define X(name) {(#name), sizeof(#name) - 1},
    LEX_ERROR_KIND_LIST(X)
    #undef X
};

StringRef TokenKind_name(TokenKind kind) {
    assert(kind < TokenKind_COUNT);
    return token_kind_names[kind];
}

StringRef TokenKind_spelling(TokenKind kind) {
    assert(kind < TokenKind_COUNT);
    return token_kind_spellings[kind];
}

StringRef LexErrorKind_name(LexErrorKind kind) {
    assert(kind < LexErrorKind_COUNT);
    return lex_error_kind_names[kind];
}

void Token_dump(Token const* token, Writer* writer) {
    Writer_print(
        writer,
        "Token(kind = .%s, position = <%u:%u>",
        TokenKind_name(token->kind),
        token->pos.line,
        token->pos.column
    );

    switch (token->kind) {
    case TokenKind_IntLiteral:
        Writer_print(writer, ", value = ");
        BigInt_write(writer, token->value.integer, 10);
        break;

    case TokenKind_Identifier:
        Writer_print(writer, ", value = \"");
        Writer_write_str(writer, token->value.string);
        Writer_print(writer, "\"");
        break;

    default:
        break;
    }

    Writer_print(writer, ")\n");
}
