#include "src/driver/diagnostics.h"
#include "src/sema/type_checking.h"

static void write_token_kind_description(Writer* writer, TokenKind kind) {
    switch (kind) {
    case TokenKind_EndOfFile:
        Writer_write_zstr(writer, "end of file");
        break;
    case TokenKind_Identifier:
        Writer_write_zstr(writer, "identifier");
        break;
    case TokenKind_IntLiteral:
        Writer_write_zstr(writer, "integer literal");
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

void report_lex_error(
    DiagnosticEngine* diagnostics,
    StringRef path,
    LexError const* error,
    DiagnosticLevel level
) {
    DiagnosticBuilder* diag;
    Writer* writer;

    diag = DiagnosticEngine_start_diagnostic(diagnostics);
    writer = DiagnosticBuilder_get_writer(diag);

    DiagnosticBuilder_set_level(diag, level);
    DiagnosticBuilder_set_category(diag, DiagnosticCategory_Tokenize);
    DiagnosticBuilder_set_source(diag, path);

    switch (error->kind) {
    case LexErrorKind_BadEncoding:
        Writer_write_zstr(writer, "invalid UTF-8 encoding");
        break;
    case LexErrorKind_LineLimitExceeded:
        Writer_write_zstr(writer, "too many lines in source file");
        break;
    case LexErrorKind_ColumnLimitExceeded:
        Writer_write_zstr(writer, "too many characters in line");
        break;
    case LexErrorKind_CharacterLimitExceeded:
        Writer_write_zstr(writer, "too many characters in file");
        break;
    case LexErrorKind_UnclosedBlockComment:
        Writer_write_zstr(writer, "unclosed block comment");
        break;
    case LexErrorKind_DecimalLeadingZero:
        Writer_write_zstr(writer, "decimal literal cannot have leading zero");
        break;
    case LexErrorKind_BadIntLiteral:
        Writer_write_zstr(writer, "invalid integer literal");
        break;
    case LexErrorKind_UnexpectedCharacter:
        Writer_format(
            writer, "unexpected character U+%04X", error->value.character
        );
    }

    DiagnosticBuilder_emit(diag);
}

void report_parse_error(
    DiagnosticEngine* diagnostics,
    StringRef path,
    ParseError const* error,
    DiagnosticLevel level
) {
    DiagnosticBuilder* diag;
    Writer* writer;

    diag = DiagnosticEngine_start_diagnostic(diagnostics);
    writer = DiagnosticBuilder_get_writer(diag);

    DiagnosticBuilder_set_level(diag, level);
    DiagnosticBuilder_set_category(diag, DiagnosticCategory_Parse);
    DiagnosticBuilder_set_source(diag, path);

    Writer_write_zstr(writer, "expected ");
    write_syntax_category(writer, error->expected_category);
    Writer_write_zstr(writer, ", found ");
    write_token_kind_description(writer, error->actual_token_kind);

    DiagnosticBuilder_emit(diag);
}

void report_yacc_error(DiagnosticEngine* diagnostics, ByteStringRef message) {
    DiagnosticBuilder* diag;
    Writer* writer;

    diag = DiagnosticEngine_start_diagnostic(diagnostics);
    writer = DiagnosticBuilder_get_writer(diag);

    DiagnosticBuilder_set_level(diag, DiagnosticLevel_Error);
    DiagnosticBuilder_set_category(diag, DiagnosticCategory_Parse);

    Writer_write_zstr(writer, "yacc: ");
    Writer_write_bstr(writer, message);

    DiagnosticBuilder_emit(diag);
}

void report_undeclared_name(
    DiagnosticEngine* diagnostics,
    StringRef path,
    UndeclaredName const* error,
    DiagnosticLevel level
) {
    DiagnosticBuilder* diag;
    Writer* writer;

    diag = DiagnosticEngine_start_diagnostic(diagnostics);
    writer = DiagnosticBuilder_get_writer(diag);

    DiagnosticBuilder_set_level(diag, level);
    DiagnosticBuilder_set_category(diag, DiagnosticCategory_TypeCheck);
    DiagnosticBuilder_set_source(diag, path);

    Writer_write_zstr(writer, "undeclared identifier `");
    Writer_write_str(writer, error->name);
    Writer_write_zstr(writer, "`");

    DiagnosticBuilder_emit(diag);
}

void report_type_mismatch(
    DiagnosticEngine* diagnostics,
    StringRef path,
    ExpectedType const* error,
    DiagnosticLevel level
) {
    DiagnosticBuilder* diag;
    Writer* writer;

    diag = DiagnosticEngine_start_diagnostic(diagnostics);
    writer = DiagnosticBuilder_get_writer(diag);

    DiagnosticBuilder_set_level(diag, level);
    DiagnosticBuilder_set_category(diag, DiagnosticCategory_TypeCheck);
    DiagnosticBuilder_set_source(diag, path);

    Writer_write_zstr(writer, "expected type `");
    Writer_write_str(writer, Type_name(error->expected));
    Writer_write_zstr(writer, "` but found `");
    Writer_write_str(writer, Type_name(error->actual));

    DiagnosticBuilder_emit(diag);
}
