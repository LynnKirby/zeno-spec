#ifndef _ZENO_SPEC_SRC_TOKEN_H
#define _ZENO_SPEC_SRC_TOKEN_H

#include "src/support/bigint.h"
#include "src/support/io.h"
#include "src/support/source_pos.h"
#include "src/support/stdint.h"
#include "src/support/string_ref.h"

#define SYMBOL_KIND_LIST(X)       \
    X(LeftParen, "(")             \
    X(RightParen, ")")            \
    X(LeftCurly, "{")             \
    X(RightCurly, "}")            \
    X(LeftSquare, "[")            \
    X(RightSquare, "]")           \
    X(Period, ".")                \
    X(Comma, ",")                 \
    X(Colon, ":")                 \
    X(Semicolon, ";")             \
    X(Equal, "=")                 \
    X(EqualEqual, "==")           \
    X(Exclaim, "!")               \
    X(ExclaimEqual, "!=")         \
    X(Plus, "+")                  \
    X(PlusEqual, "+=")            \
    X(Minus, "-")                 \
    X(MinusEqual, "-=")           \
    X(Star, "*")                  \
    X(StarEqual, "*=")            \
    X(Slash, "/")                 \
    X(SlashEqual, "/=")           \
    X(Percent, "%")               \
    X(PercentEqual, "%=")         \
    X(Less, "<")                  \
    X(LessEqual, "<=")            \
    X(LessLess, "<<")             \
    X(LessLessEqual, "<<=")       \
    X(Greater, "<")               \
    X(GreaterEqual, "<=")         \
    X(GreaterGreater, "<<")       \
    X(GreaterGreaterEqual, "<<=") \
    X(Tilde, "~")                 \
    X(Amp, "&")                   \
    X(AmpEqual, "&=")             \
    X(AmpAmp, "&&")               \
    X(AmpAmpEqual, "&&=")         \
    X(Bar, "|")                   \
    X(BarEqual, "|=")             \
    X(BarBar, "||")               \
    X(BarBarEqual, "||=")         \
    X(Caret, "^")                 \
    X(CaretEqual, "^=")           \
    X(CaretCaret, "^^")           \
    X(CaretCaretEqual, "^^=")     \
    X(ThinArrow, "->")            \
    X(FatArrow, "=>")             \
    X(At, "@")                    \
    X(ClosedRange, "...")         \
    X(HalfOpenRange, "..<")

#define KEYWORD_KIND_LIST(X)  \
    X(Def, "def")             \
    X(Class, "class")         \
    X(Else, "else")           \
    X(False, "false")         \
    X(For, "for")             \
    X(If, "if")               \
    X(Import, "import")       \
    X(In, "in")               \
    X(Interface, "interface") \
    X(Let, "let")             \
    X(Mut, "mut")             \
    X(Out, "out")             \
    X(Return, "return")       \
    X(True, "true")           \
    X(Var, "var")             \
    X(While, "while")

#define TOKEN_KIND_LIST(X) \
    X(EndOfFile, "")       \
    X(Identifier, "")      \
    X(IntLiteral, "")      \
    SYMBOL_KIND_LIST(X)    \
    KEYWORD_KIND_LIST(X)

typedef enum TokenKind {
    #define X(name, str) TokenKind_##name,
    TOKEN_KIND_LIST(X)
    #undef X
    TokenKind_COUNT ATTR_UNUSED
} TokenKind;

StringRef TokenKind_name(TokenKind kind);
StringRef TokenKind_spelling(TokenKind kind);

#define LEX_ERROR_KIND_LIST(X) \
    X(BadEncoding)             \
    X(UnexpectedCharacter)     \
    X(LineLimitExceeded)       \
    X(ColumnLimitExceeded)     \
    X(CharacterLimitExceeded)  \
    X(UnclosedBlockComment)    \
    X(DecimalLeadingZero)      \
    X(BadIntLiteral)

typedef enum LexErrorKind {
    #define X(name) LexErrorKind_##name,
    LEX_ERROR_KIND_LIST(X)
    #undef X
    LexErrorKind_COUNT ATTR_UNUSED
} LexErrorKind;

StringRef LexErrorKind_name(LexErrorKind kind);

typedef struct LexError {
    SourcePos pos;
    LexErrorKind kind;
    union {
        int32_t character;
    } value;
} LexError;

typedef struct Token {
    SourcePos pos;
    TokenKind kind;
    union {
        /* Owned. */
        BigInt integer;
        /* Borrowed from Lexer. */
        StringRef string;
    } value;
} Token;


typedef struct TokenList {
    Token* data;
    size_t size;
} TokenList;

void Token_dump(Token const* token, Writer* writer);

#endif
