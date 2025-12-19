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

static void report_multiple_input_files(DiagnosticEngine* diagnostics) {
    DiagnosticBuilder* diag;
    Writer* writer;

    diag = DiagnosticEngine_start_diagnostic(diagnostics);
    writer = DiagnosticBuilder_get_writer(diag);

    DiagnosticBuilder_set_level(diag, DiagnosticLevel_Error);
    DiagnosticBuilder_set_category(diag, DiagnosticCategory_Driver);

    Writer_write_zstr(writer, "multiple input files");

    DiagnosticBuilder_emit(diag);
}

static void report_unknown_flag(DiagnosticEngine* diagnostics, StringRef flag) {
    DiagnosticBuilder* diag;
    Writer* writer;

    diag = DiagnosticEngine_start_diagnostic(diagnostics);
    writer = DiagnosticBuilder_get_writer(diag);

    DiagnosticBuilder_set_level(diag, DiagnosticLevel_Error);
    DiagnosticBuilder_set_category(diag, DiagnosticCategory_Driver);

    Writer_write_zstr(writer, "unknown flag `");
    Writer_write_str(writer, flag);
    Writer_write_zstr(writer, "`");

    DiagnosticBuilder_emit(diag);
}

static void report_no_input_file(DiagnosticEngine* diagnostics) {
    DiagnosticBuilder* diag;
    Writer* writer;

    diag = DiagnosticEngine_start_diagnostic(diagnostics);
    writer = DiagnosticBuilder_get_writer(diag);

    DiagnosticBuilder_set_level(diag, DiagnosticLevel_Error);
    DiagnosticBuilder_set_category(diag, DiagnosticCategory_Driver);

    Writer_write_zstr(writer, "no input file");

    DiagnosticBuilder_emit(diag);
}

static void report_tokenize_unexpected_success(DiagnosticEngine* diagnostics) {
    DiagnosticBuilder* diag;
    Writer* writer;

    diag = DiagnosticEngine_start_diagnostic(diagnostics);
    writer = DiagnosticBuilder_get_writer(diag);

    DiagnosticBuilder_set_level(diag, DiagnosticLevel_Error);
    DiagnosticBuilder_set_category(diag, DiagnosticCategory_Driver);

    Writer_write_zstr(writer, "expected tokenize error");

    DiagnosticBuilder_emit(diag);
}

static void report_parse_unexpected_success(DiagnosticEngine* diagnostics) {
    DiagnosticBuilder* diag;
    Writer* writer;

    diag = DiagnosticEngine_start_diagnostic(diagnostics);
    writer = DiagnosticBuilder_get_writer(diag);

    DiagnosticBuilder_set_level(diag, DiagnosticLevel_Error);
    DiagnosticBuilder_set_category(diag, DiagnosticCategory_Driver);

    Writer_write_zstr(writer, "expected parse error");

    DiagnosticBuilder_emit(diag);
}

static void report_check_unexpected_success(DiagnosticEngine* diagnostics) {
    DiagnosticBuilder* diag;
    Writer* writer;

    diag = DiagnosticEngine_start_diagnostic(diagnostics);
    writer = DiagnosticBuilder_get_writer(diag);

    DiagnosticBuilder_set_level(diag, DiagnosticLevel_Error);
    DiagnosticBuilder_set_category(diag, DiagnosticCategory_Driver);

    Writer_write_zstr(writer, "expected static checking error");

    DiagnosticBuilder_emit(diag);
}

static void report_open_error(
    DiagnosticEngine* diagnostics, StringRef path, SystemIoError error
) {
    DiagnosticBuilder* diag;
    Writer* writer;

    diag = DiagnosticEngine_start_diagnostic(diagnostics);
    writer = DiagnosticBuilder_get_writer(diag);

    DiagnosticBuilder_set_level(diag, DiagnosticLevel_Error);
    DiagnosticBuilder_set_category(diag, DiagnosticCategory_System);

    Writer_write_zstr(writer, "could not open `");
    Writer_write_str(writer, path);
    Writer_write_zstr(writer, "`; system error ");
    Writer_write_int(writer, error, 10);

    DiagnosticBuilder_emit(diag);
}

static void report_read_error(
    DiagnosticEngine* diagnostics, StringRef path, SystemIoError error
) {
    DiagnosticBuilder* diag;
    Writer* writer;

    diag = DiagnosticEngine_start_diagnostic(diagnostics);
    writer = DiagnosticBuilder_get_writer(diag);

    DiagnosticBuilder_set_level(diag, DiagnosticLevel_Error);
    DiagnosticBuilder_set_category(diag, DiagnosticCategory_System);

    Writer_write_zstr(writer, "could not read `");
    Writer_write_str(writer, path);
    Writer_write_zstr(writer, "`; system error ");
    Writer_write_int(writer, error, 10);

    DiagnosticBuilder_emit(diag);
}

static void parse_options(
    Options* options,
    DiagnosticEngine* diagnostics,
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
                report_multiple_input_files(diagnostics);
                return;
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
                report_unknown_flag(diagnostics, arg);
                return;
            }
        }

        argv += 1;
        argc -= 1;
    }

    if (!has_path) {
        report_no_input_file(diagnostics);
    }
}

static void dump_tokens(Writer* writer, TokenList const* tokens) {
    size_t i;
    for (i = 0; i < tokens->size; i += 1) {
        Token_dump(&tokens->data[i], writer);
    }
}

static void do_check(
    DiagnosticEngine* diagnostics,
    Options const* options,
    AstContext* ast,
    FunctionItem* item
) {
    TypeCheckResult check_result;
    DiagnosticLevel error_level = DiagnosticLevel_Error;

    if (options->expect_failure) {
        if (options->quiet) {
            error_level = DiagnosticLevel_Ignore;
        } else {
            error_level = DiagnosticLevel_Info;
        }
    }

    type_check(&check_result, ast, item);

    switch (check_result.kind) {
    case TypeCheckResultKind_Success:
        if (options->expect_failure) {
            report_check_unexpected_success(diagnostics);
        }
        break;

    case TypeCheckResultKind_UndeclaredName:
        report_undeclared_name(
            diagnostics,
            options->path,
            &check_result.as.undeclared_name,
            error_level
        );
        break;

    case TypeCheckResultKind_ExpectedType:
        report_type_mismatch(
            diagnostics,
            options->path,
            &check_result.as.expected_type,
            error_level
        );
        break;
    }
}

static void do_parse(
    DiagnosticEngine* diagnostics,
    Options const* options,
    AstContext* ast,
    TokenList const* tokens,
    Command command
) {
    ParseResult parse_result;
    DiagnosticLevel error_level = DiagnosticLevel_Error;

    if (command == Command_Parse && options->expect_failure) {
        if (options->quiet) {
            error_level = DiagnosticLevel_Ignore;
        } else {
            error_level = DiagnosticLevel_Info;
        }
    }

    parse(&parse_result, ast, tokens);

    switch (parse_result.kind) {
    case ParseResultKind_Success:
        if (command == Command_Check) {
            do_check(
                diagnostics, options, ast, (FunctionItem*)parse_result.u.item
            );
        } else {
            if (!options->quiet && !options->expect_failure) {
                FunctionItem_dump(
                    (FunctionItem*)parse_result.u.item, Writer_stdout
                );
            }
            if (options->expect_failure) {
                report_parse_unexpected_success(diagnostics);
            }
        }
        break;

    case ParseResultKind_ParseError:
        report_parse_error(
            diagnostics,
            options->path,
            &parse_result.u.parse_error,
            error_level
        );
        break;

    case ParseResultKind_YaccError:
        report_yacc_error(diagnostics, parse_result.u.yacc_error);
        break;
    }
}

static void do_syntax(
    DiagnosticEngine* diagnostics,
    Options const* options,
    AstContext* ast,
    SystemFile file,
    Command command
) {
    LexResult lex_result;
    SystemIoError io_res;
    SourceFile const* source;
    DiagnosticLevel error_level = DiagnosticLevel_Error;

    if (command == Command_Tokenize && options->expect_failure) {
        if (options->quiet) {
            error_level = DiagnosticLevel_Ignore;
        } else {
            error_level = DiagnosticLevel_Info;
        }
    }

    io_res = AstContext_source_from_file(ast, options->path, file, &source);

    if (io_res != SystemIoError_Success) {
        report_read_error(diagnostics, options->path, io_res);
        return;
    }

    lex_source(&lex_result, ast, source, NULL);

    if (lex_result.is_tokens) {
        if (command == Command_Tokenize) {
            if (!options->quiet && !options->expect_failure) {
                dump_tokens(Writer_stdout, &lex_result.u.tokens);
            }
            if (options->expect_failure) {
                report_tokenize_unexpected_success(diagnostics);
            }
        } else {
            do_parse(
                diagnostics, options, ast, &lex_result.u.tokens, command
            );
        }

        xfree(lex_result.u.tokens.data);
    } else {
        report_lex_error(
            diagnostics,
            options->path,
            &lex_result.u.error,
            error_level
        );
    }
}

static void do_command(
    DiagnosticEngine* diagnostics,
    int argc,
    char const* const* argv,
    Command command
) {
    Options options;
    AstContext* ast;

    parse_options(&options, diagnostics, argc, argv);

    if (DiagnosticEngine_has_errors(diagnostics)) {
        return;
    }

    ast = AstContext_new();

    if (StringRef_equal_zstr(options.path, "-")) {
        options.path = StringRef_from_zstr("<stdin>");
        do_syntax(diagnostics, &options, ast, SystemFile_stdin, command);
    } else {
        SystemFile file;
        SystemIoError io_res;

        io_res = SystemFile_open_read(&file, (char const*)options.path.data);

        if (io_res == SystemIoError_Success) {
            do_syntax(diagnostics, &options, ast, file, command);
            SystemFile_close(file);
        } else {
            report_open_error(diagnostics, options.path, io_res);
        }
    }

    AstContext_delete(ast);
}

void tokenize_command(
    DiagnosticEngine* diagnostics, int argc, char const* const* argv
) {
    do_command(diagnostics, argc, argv, Command_Tokenize);
}

void parse_command(
    DiagnosticEngine* diagnostics, int argc, char const* const* argv
) {
    do_command(diagnostics, argc, argv, Command_Parse);
}

void check_command(
    DiagnosticEngine* diagnostics, int argc, char const* const* argv
) {
    do_command(diagnostics, argc, argv, Command_Check);
}
