#include "src/driver/diagnostics.h"
#include "src/lang/binding.h"

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

void write_error_prefix(Writer* writer, ByteStringRef path, SourcePos pos) {
    Writer_write_bstr(writer, path);
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
    Writer* writer, ByteStringRef path, LexError const* error
) {
    write_error_prefix(writer, path, error->pos);
    write_lex_error_description(writer, error);
    Writer_format(writer, "\n");
}

void write_parse_error(
    Writer* writer, ByteStringRef path, ParseError const* error
) {
    write_error_prefix(writer, path, error->token_pos);
    Writer_format(writer, "unexpected ");
    write_token_kind_description(writer, error->token_kind);
    Writer_format(writer, "\n");
}

void write_yacc_error(Writer* writer, ByteStringRef message) {
    Writer_format(writer, "zeno-spec: error: yacc: ");
    Writer_write_bstr(writer, message);
    Writer_format(writer, "\n");
}

void write_undefined_identifier_error(
    Writer* writer, ByteStringRef path, UndefinedIdentifier* error
) {
    write_error_prefix(writer, path, error->pos);
    Writer_format(writer, "undefined identifier `");
    Writer_write_str(writer, error->value);
    Writer_format(writer, "`\n");
}
