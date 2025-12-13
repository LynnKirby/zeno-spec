#include "src/io.h"
#include "src/lex.h"

#include <stdlib.h>
#include <string.h>

static void note(char const* format, ...) {
    va_list args;
    va_start(args, format);
    Writer_print(Writer_stderr, "zeno-spec: note: ");
    Writer_vprint(Writer_stderr, format, args);
    Writer_print(Writer_stderr, "\n");
    va_end(args);
}

static void error(char const* format, ...) {
    va_list args;
    va_start(args, format);
    Writer_print(Writer_stderr, "zeno-spec: error: ");
    Writer_vprint(Writer_stderr, format, args);
    Writer_print(Writer_stderr, "\n");
    va_end(args);
}

static int print_tokens(ByteStringRef source) {
    int res = 0;
    Lexer lexer;
    LexResult token_or_error;
    Lexer_init(&lexer, source, NULL);

    while (Lexer_next(&lexer, &token_or_error)) {
        if (token_or_error.is_token) {
            Token token;
            token = token_or_error.u.token;
            Writer_print(Writer_stdout, "%s", TokenKind_name(token.kind));
            if (token.kind == TokenKind_IntLiteral) {
                Writer_print(Writer_stdout, " value=%u", token.value.integer);
            }
            Writer_print(
                Writer_stdout, " position=%u:%u\n", token.line, token.column
            );
        } else {
            LexError error;
            error = token_or_error.u.error;
            Writer_print(
                Writer_stdout, "%s", LexErrorKind_name(error.kind)
            );
            if (error.kind == LexErrorKind_UnexpectedCharacter) {
                Writer_print(
                    Writer_stdout, " value=0x%04X", error.value.character
                );
            }
            Writer_print(
                Writer_stdout, " position=%u:%u\n", error.line, error.column
            );
            res = 1;
        }
    }

    Lexer_destroy(&lexer);
    return res;
}

int main(int argc, char const* const* argv) {
    int res;
    char const* path;
    char* file_buf = NULL;
    size_t file_size;
    size_t file_buf_capacity;
    SystemFile file;

    if (argc < 2) {
        error("no input files");
        return 1;
    }

    if (argc > 2) {
        error("multiple input files not supported yet");
        return 1;
    }

    path = argv[1];

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

    file_buf = malloc(1024);
    file_size = 0;
    file_buf_capacity = 1024;

    for (;;) {
        SystemIoError res;
        size_t chunk_read;
        char chunk[1024];

        res = SystemFile_read(file, chunk, sizeof(chunk), &chunk_read);
        if (res != SystemIoError_Success) {
            error("could not read '%s': system error XX", path);
            free(file_buf);
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
                file_buf = realloc(file_buf, file_buf_capacity);
            }
        }
    }

    /* Nul terminate. */
    if ((file_buf_capacity - file_size) == 0) {
        file_buf = realloc(file_buf, file_size + 1);
    }
    file_buf[file_size] = 0;

    {
        ByteStringRef source;
        source.data = file_buf;
        source.size = file_size + 1;
        res = print_tokens(source);
    }

    free(file_buf);

    return res;
}
