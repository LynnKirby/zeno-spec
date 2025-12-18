#include "src/driver/diagnostics.h"
#include "src/sema/type_checking.h"

static void write_lex_error_description(Writer* writer, LexError const* error) {
    switch (error->kind) {
    case LexErrorKind_BadEncoding:
        Writer_format(writer, "invalid UTF-8 encoding");
        break;
    case LexErrorKind_LineLimitExceeded:
        Writer_format(writer, "too many lines in source file");
        break;
    case LexErrorKind_ColumnLimitExceeded:
        Writer_format(writer, "too many characters in line");
        break;
    case LexErrorKind_CharacterLimitExceeded:
        Writer_format(writer, "too many characters in file");
        break;
    case LexErrorKind_UnclosedBlockComment:
        Writer_format(writer, "unclosed block comment");
        break;
    case LexErrorKind_DecimalLeadingZero:
        Writer_format(writer, "decimal literal cannot have leading zero");
        break;
    case LexErrorKind_BadIntLiteral:
        Writer_format(writer, "invalid integer literal");
        break;
    case LexErrorKind_UnexpectedCharacter:
        Writer_format(
            writer, "unexpected character U+%04X", error->value.character
        );
    }
}

static void write_token_kind_description(Writer* writer, TokenKind kind) {
    switch (kind) {
    case TokenKind_EndOfFile:
        Writer_format(writer, "end of file");
        break;
    case TokenKind_Identifier:
        Writer_format(writer, "identifier");
        break;
    case TokenKind_IntLiteral:
        Writer_format(writer, "integer literal");
        break;
    #define CASE(name, str) case TokenKind_##name:
    SYMBOL_KIND_LIST(CASE)
        Writer_format(writer, "symbol `%s`", TokenKind_spelling(kind));
        break;
    KEYWORD_KIND_LIST(CASE)
        Writer_format(writer, "keyword `%s`", TokenKind_spelling(kind));
        break;
    #undef CASE
    }
}

static void write_syntax_category(Writer* writer, SyntaxCategory category) {
    switch (category) {
    #define X(name, str)                \
        case SyntaxCategory_##name:     \
            Writer_format(writer, str); \
            break;
    SYNTAX_CATEGORY_LIST(X)
    #undef X
    }
}

void write_error_prefix(Writer* writer, StringRef path, SourcePos pos) {
    Writer_write_str(writer, path);
    Writer_format(writer, ":");

    if (pos.line > 0) {
        Writer_format(writer, "%u:", pos.line);
        if (pos.column > 0) {
            Writer_format(writer, "%u:", pos.column);
        }
    }

    Writer_format(writer, " error: ");
}

void write_lex_error(
    Writer* writer, StringRef path, LexError const* error
) {
    write_error_prefix(writer, path, error->pos);
    write_lex_error_description(writer, error);
    Writer_format(writer, "\n");
}

void write_parse_error(
    Writer* writer, StringRef path, ParseError const* error
) {
    write_error_prefix(writer, path, error->actual_token_pos);
    Writer_format(writer, "expected ");
    write_syntax_category(writer, error->expected_category);
    Writer_format(writer, ", found ");
    write_token_kind_description(writer, error->actual_token_kind);
    Writer_format(writer, "\n");
}

void write_yacc_error(Writer* writer, ByteStringRef message) {
    Writer_format(writer, "zeno-spec: error: yacc: ");
    Writer_write_bstr(writer, message);
    Writer_format(writer, "\n");
}

void write_undeclared_name_error(
    Writer* writer, StringRef path, UndeclaredName const* error
) {
    write_error_prefix(writer, path, error->pos);
    Writer_format(writer, "undefined identifier `");
    Writer_write_str(writer, error->name);
    Writer_format(writer, "`\n");
}

void write_expected_type_error(
    Writer* writer, StringRef path, ExpectedType const* error
) {
    write_error_prefix(writer, path, error->pos);
    Writer_format(writer, "expected type `");
    Writer_write_str(writer, Type_name(error->expected));
    Writer_format(writer, "` but found `");
    Writer_write_str(writer, Type_name(error->actual));
    Writer_format(writer, "`\n");
}
