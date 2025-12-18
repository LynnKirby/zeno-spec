#include "src/driver/commands.h"
#include "src/parsing/lex.h"
#include "src/parsing/parse.h"
#include "src/support/malloc.h"
#include "src/driver/diagnostics.h"
#include "src/ast/dump.h"
#include "src/sema/type_checking.h"

#include <assert.h>

typedef enum Command {
    Command_Tokenize,
    Command_Parse,
    Command_Check
} Command;

typedef struct Options {
    StringRef path;
    int quiet;
    int expect_failure;
} Options;

static int parse_options(
    Options* options,
    StringRef progname,
    int argc,
    char const* const* argv
) {
    int has_path = false;
    int dash_dash = false;

    options->quiet = false;
    options->expect_failure = false;

    while (argc > 0) {
        StringRef arg;

        arg = StringRef_from_zstr(*argv);

        if (
            dash_dash
            || arg.data[0] != '-'
            || (arg.size == 1 && arg.data[0] == '-')
        ) {
            /* Positional */
            if (has_path) {
                Writer_write_str(Writer_stderr, progname);
                Writer_format(
                    Writer_stderr, ": error: multiple input files\n"
                );
                return 1;
            }

            options->path = arg;
            has_path = true;
        } else if (arg.size == 2 && arg.data[0] == '-' && arg.data[1] == '-') {
            dash_dash = true;
        } else {
            static StringRef quiet_flag = STATIC_STRING_REF("--quiet");
            static StringRef expect_failure_flag =
                STATIC_STRING_REF("--expect-failure");

            if (StringRef_equal(arg, quiet_flag)) {
                options->quiet = true;
            } else if (StringRef_equal(arg, expect_failure_flag)) {
                options->expect_failure = true;
            } else {
                Writer_write_str(Writer_stderr, progname);
                Writer_format(
                    Writer_stderr, ": error: unknown flag `%s`\n", arg.data
                );
                return 1;
            }
        }

        argv += 1;
        argc -= 1;
    }

    if (!has_path) {
        Writer_write_str(Writer_stderr, progname);
        Writer_format(Writer_stderr, ": error: no input file\n");
    }

    return 0;
}

static void dump_tokens(Writer* writer, TokenList const* tokens) {
    size_t i;
    for (i = 0; i < tokens->size; i += 1) {
        Token_dump(&tokens->data[i], writer);
    }
}

static int do_check(
    StringRef progname,
    Options const* options,
    AstContext* ast,
    FunctionItem* item
) {
    int res = 0;
    TypeCheckResult check_result;

    (void)progname;

    type_check(&check_result, ast, item);

    switch (check_result.kind) {
    case TypeCheckResultKind_Success:
        if (options->expect_failure) {
            Writer_write_str(Writer_stderr, options->path);
            Writer_format(
                Writer_stderr, ": error: check was expected to fail\n"
            );
            res = 1;
        }
        break;

    case TypeCheckResultKind_UndeclaredName:
        if (!options->expect_failure) {
            write_undeclared_name_error(
                Writer_stderr, options->path, &check_result.as.undeclared_name
            );
            res = 1;
        }
        break;

    case TypeCheckResultKind_ExpectedType:
        if (!options->expect_failure) {
            write_expected_type_error(
                Writer_stderr, options->path, &check_result.as.expected_type
            );
            res = 1;
        }
        break;
    }

    return res;
}

static int do_parse(
    StringRef progname,
    Options const* options,
    AstContext* ast,
    TokenList const* tokens,
    Command command
) {
    int res = 0;
    ParseResult parse_result;

    (void)progname;

    parse(&parse_result, ast, tokens);

    switch (parse_result.kind) {
    case ParseResultKind_Success:
        if (command == Command_Check) {
            res = do_check(
                progname, options, ast, (FunctionItem*)parse_result.u.item
            );
        } else {
            if (!options->quiet) {
                FunctionItem_dump(
                    (FunctionItem*)parse_result.u.item, Writer_stdout
                );
            }

            if (options->expect_failure) {
                Writer_write_str(Writer_stderr, options->path);
                Writer_format(
                    Writer_stderr, ": error: parse was expected to fail\n"
                );
                res = 1;
            }
        }
        break;

    case ParseResultKind_ParseError:
        if (command != Command_Parse || !options->expect_failure) {
            write_parse_error(
                Writer_stderr, options->path, &parse_result.u.parse_error
            );
            res = 1;
        }
        break;

    case ParseResultKind_YaccError:
        write_yacc_error(Writer_stderr, parse_result.u.yacc_error);
        res = 1;
        break;
    }

    return res;
}

static int do_syntax(
    StringRef progname,
    Options const* options,
    AstContext* ast,
    SystemFile file,
    Command command
) {
    int res = 0;
    LexResult lex_result;
    SystemIoError io_res;
    SourceFile const* source;

    io_res = AstContext_source_from_file(ast, options->path, file, &source);

    if (io_res != SystemIoError_Success) {
        Writer_write_str(Writer_stderr, progname);
        Writer_format(
            Writer_stderr,
            ": error: cannot read '%s', system error %u\n",
            options->path.data,
            io_res
        );
        return 1;
    }

    lex_source(&lex_result, ast, source, NULL);

    if (lex_result.is_tokens) {
        if (command == Command_Tokenize) {
            if (!options->quiet) {
                dump_tokens(Writer_stdout, &lex_result.u.tokens);
            }

            if (options->expect_failure) {
                Writer_write_str(Writer_stderr, options->path);
                Writer_format(
                    Writer_stderr, ": error: tokenize was expected to fail\n"
                );
                res = 1;
            }
        } else {
            res = do_parse(
                progname, options, ast, &lex_result.u.tokens, command
            );
        }

        xfree(lex_result.u.tokens.data);
    } else {
        if (command != Command_Tokenize || !options->expect_failure) {
            write_lex_error(Writer_stderr, options->path, &lex_result.u.error);
            res = 1;
        }
    }

    return res;
}

static int do_command(
    StringRef progname, int argc, char const* const* argv, Command command
) {
    int res;
    Options options;
    AstContext* ast;

    res = parse_options(&options, progname, argc, argv);
    if (res != 0) return res;

    ast = AstContext_new();

    if (StringRef_equal_zstr(options.path, "-")) {
        options.path = StringRef_from_zstr("<stdin>");
        res = do_syntax(progname, &options, ast, SystemFile_stdin, command);
    } else {
        SystemFile file;
        SystemIoError io_res;

        io_res = SystemFile_open_read(&file, (char const*)options.path.data);

        if (io_res == SystemIoError_Success) {
            res = do_syntax(progname, &options, ast, file, command);
            SystemFile_close(file);
        } else {
            Writer_write_str(Writer_stderr, progname);
            Writer_format(
                Writer_stderr,
                ": error: cannot open '%s', system error %u\n",
                options.path.data,
                io_res
            );
            res = 1;
        }
    }

    AstContext_delete(ast);

    return res;
}

int tokenize_command(StringRef progname, int argc, char const* const* argv) {
    return do_command(progname, argc, argv, Command_Tokenize);
}

int parse_command(StringRef progname, int argc, char const* const* argv) {
    return do_command(progname, argc, argv, Command_Parse);
}

int check_command(StringRef progname, int argc, char const* const* argv) {
    return do_command(progname, argc, argv, Command_Check);
}
