#include "src/lang/lex.h"
#include "src/lang/parse.h"
#include "src/support/io.h"
#include "src/support/malloc.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

static void note(char const* format, ...) {
    va_list args;
    va_start(args, format);
    Writer_format(Writer_stderr, "zeno-spec: note: ");
    Writer_vformat(Writer_stderr, format, args);
    Writer_format(Writer_stderr, "\n");
    va_end(args);
}

static void error(char const* format, ...) {
    va_list args;
    va_start(args, format);
    Writer_format(Writer_stderr, "zeno-spec: error: ");
    Writer_vformat(Writer_stderr, format, args);
    Writer_format(Writer_stderr, "\n");
    va_end(args);
}

static void print_token_kind_description(Writer* writer, TokenKind kind) {
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

static void print_lex_error_description(Writer* writer, LexError const* error) {
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

static void print_lex_error(
    Writer* writer, ByteStringRef path, LexError const* error
) {
    Writer_write_bstr(writer, path);
    Writer_format(writer, ":%u:%u: error: ", error->pos.line, error->pos.column);
    print_lex_error_description(writer, error);
    Writer_format(writer, "\n");
}

static void print_parse_error(
    Writer* writer, ByteStringRef path, ParseError const* error
) {
    Writer_write_bstr(writer, path);
    Writer_format(
        writer,
        ":%u:%u: error: unexpected ",
        error->token_pos.line,
        error->token_pos.column
    );
    print_token_kind_description(writer, error->token_kind);
    Writer_format(writer, "\n");
}

static void print_yacc_error(Writer* writer, ByteStringRef message) {
    Writer_format(writer, "zeno-spec: error: yacc: ");
    Writer_write_bstr(writer, message);
    Writer_format(writer, "\n");
}

typedef enum DriverAction {
    DriverAction_Unknown,
    DriverAction_DumpLex,
    DriverAction_CheckLex,
    DriverAction_CheckLexInvalid,
    DriverAction_DumpParse,
    DriverAction_CheckParse,
    DriverAction_CheckParseInvalid
} DriverAction;

static int lex_action(
    ByteStringRef path, ByteStringRef source, DriverAction action
) {
    int lex_okay = true;
    Lexer lexer;
    LexResult lex_result;

    Lexer_init(&lexer, source, NULL);

    while (Lexer_next(&lexer, &lex_result)) {
        if (lex_result.is_token) {
            if (action == DriverAction_DumpLex) {
                Token_dump(&lex_result.u.token, Writer_stdout);
            }
        } else {
            if (action != DriverAction_CheckLexInvalid) {
                print_lex_error(Writer_stderr, path, &lex_result.u.error);
            }
            lex_okay = false;
            break;
        }
    }

    Lexer_destroy(&lexer);

    switch (action) {
    case DriverAction_DumpLex:
        return lex_okay ? 0 : 1;
    case DriverAction_CheckLex:
        return lex_okay ? 0 : 1;
    case DriverAction_CheckLexInvalid:
        return lex_okay ? 1 : 0;
    default:
        assert(0 && "unreachable");
    }
}

static int syntax_action(
    ByteStringRef path, ByteStringRef source, DriverAction action
) {
    int res = 0;
    AstContext context;
    Lexer lexer;
    ParseResult parse_result;

    AstContext_init(&context);
    Lexer_init(&lexer, source, NULL);

    parse(&parse_result, &context, &lexer);

    switch (parse_result.kind) {
    case ParseResultKind_Success:
        if (action == DriverAction_DumpParse) {
            Item_dump(parse_result.u.syntax, Writer_stdout);
        } else if (action == DriverAction_CheckParseInvalid) {
            res = 1;
        }
        break;
    case ParseResultKind_LexError:
        if (action != DriverAction_CheckParseInvalid) {
            print_lex_error(Writer_stderr, path, &parse_result.u.lex_error);
            res = 1;
        }
        break;
    case ParseResultKind_ParseError:
        if (action != DriverAction_CheckParseInvalid) {
            print_parse_error(Writer_stderr, path, &parse_result.u.parse_error);
            res = 1;
        }
        break;
    case ParseResultKind_YaccError:
        print_yacc_error(Writer_stderr, parse_result.u.yacc_error);
        res = 1;
        break;
    }

    Lexer_destroy(&lexer);
    AstContext_destroy(&context);
    return res;
}

int main(int argc, char const* const* argv) {
    int res;
    char const* path = NULL;
    char* file_buf = NULL;
    size_t file_size;
    size_t file_buf_capacity;
    SystemFile file;
    DriverAction action = DriverAction_Unknown;

    {
        int i;
        for (i = 1; i < argc; i += 1) {
            char const* arg;
            arg = argv[i];
            if (arg[0] != '-' || arg[1] == 0) {
                path = arg;
                continue;
            }
            if (strcmp(arg, "--dump-lex") == 0) {
                action = DriverAction_DumpLex;
                continue;
            }
            if (strcmp(arg, "--check-lex") == 0) {
                action = DriverAction_CheckLex;
                continue;
            }
            if (strcmp(arg, "--check-lex-invalid") == 0) {
                action = DriverAction_CheckLexInvalid;
                continue;
            }
            if (strcmp(arg, "--dump-parse") == 0) {
                action = DriverAction_DumpParse;
                continue;
            }
            if (strcmp(arg, "--check-parse") == 0) {
                action = DriverAction_CheckParse;
                continue;
            }
            if (strcmp(arg, "--check-parse-invalid") == 0) {
                action = DriverAction_CheckParseInvalid;
                continue;
            }
            Writer_format(
                Writer_stderr, "zeno-spec: error: unknown flag '%s'\n", arg
            );
            return 1;
        }
    }

    if (path == NULL) {
        error("no input files");
        return 1;
    }

    if (action == DriverAction_Unknown) {
        error("action not specified");
        return 1;
    }

    if (path[0] == '-' && path[1] == 0) {
        path = "<stdin>";
        file = SystemFile_stdin;

        if (SystemFile_isatty(file)) {
            note("reading from stdin");
        }
    } else {
        SystemIoError res;
        res = SystemFile_open_read(&file, path);
        if (res != SystemIoError_Success) {
            error("could not open '%s': system error XX", path);
            return 1;
        }
    }

    file_buf = xmalloc(1024);
    file_size = 0;
    file_buf_capacity = 1024;

    for (;;) {
        SystemIoError res;
        size_t chunk_read;
        char chunk[1024];

        res = SystemFile_read(file, chunk, sizeof(chunk), &chunk_read);
        if (res != SystemIoError_Success) {
            error("could not read '%s': system error XX", path);
            xfree(file_buf);
            return 1;
        }

        if (chunk_read == 0) break;

        for (;;) {
            size_t buf_available;
            buf_available = file_buf_capacity - file_size;
            if (buf_available >= chunk_read) {
                memcpy(file_buf + file_size, chunk, chunk_read);
                file_size += chunk_read;
                break;
            } else {
                memcpy(file_buf + file_size, chunk, buf_available);
                file_buf_capacity += file_buf_capacity / 2; /* FIXME: overflow */
                file_buf = xrealloc(file_buf, file_buf_capacity);
            }
        }
    }

    /* Nul terminate. */
    if ((file_buf_capacity - file_size) == 0) {
        file_buf = realloc(file_buf, file_size + 1);
    }
    file_buf[file_size] = 0;

    {
        ByteStringRef file_ref;
        ByteStringRef source;

        source.data = file_buf;
        source.size = file_size + 1;

        file_ref.data = path;
        file_ref.size = strlen(path);

        switch (action) {
        case DriverAction_DumpLex:
        case DriverAction_CheckLex:
        case DriverAction_CheckLexInvalid:
            res = lex_action(file_ref, source, action);
            break;
        case DriverAction_DumpParse:
        case DriverAction_CheckParse:
        case DriverAction_CheckParseInvalid:
            res = syntax_action(file_ref, source, action);
            break;
        case DriverAction_Unknown:
            assert(0 && "unreachable");
        }
    }

    xfree(file_buf);

    return res;
}
