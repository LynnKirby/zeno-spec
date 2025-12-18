#include "src/driver/actions.h"
#include "src/driver/diagnostics.h"
#include "src/support/io.h"
#include "src/support/string_ref.h"

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

int main(int argc, char const* const* argv) {
    int res;
    StringRef path = {0};
    SystemFile file;
    DriverAction action = DriverAction_Unknown;

    {
        int i;
        for (i = 1; i < argc; i += 1) {
            char const* arg;
            arg = argv[i];
            if (arg[0] != '-' || arg[1] == 0) {
                path = StringRef_from_zstr(arg);
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
            if (strcmp(arg, "--dump-types") == 0) {
                assert(0 && "not implemented");
            }
            if (strcmp(arg, "--check-types") == 0) {
                action = DriverAction_CheckTypes;
                continue;
            }
            if (strcmp(arg, "--check-types-invalid") == 0) {
                action = DriverAction_CheckTypesInvalid;
                continue;
            }
            Writer_format(
                Writer_stderr, "zeno-spec: error: unknown flag '%s'\n", arg
            );
            return 1;
        }
    }

    if (path.size == 0) {
        error("no input files");
        return 1;
    }

    if (action == DriverAction_Unknown) {
        error("action not specified");
        return 1;
    }

    if (path.size == 1 && path.data[0] == '-') {
        path = StringRef_from_zstr("<stdin>");
        file = SystemFile_stdin;

        if (SystemFile_isatty(file)) {
            note("reading from stdin");
        }
    } else {
        SystemIoError io_res;
        io_res = SystemFile_open_read(&file, (char const*)path.data); /*FIXME*/
        if (io_res != SystemIoError_Success) {
            error("could not open '%s': system error %u", path, io_res);
            return 1;
        }
    }

    {
        AstContext* ast;
        SystemIoError io_res;
        SourceFile const* source;

        ast = AstContext_new();

        io_res = AstContext_source_from_file(ast, path, file, &source);

        if (io_res != SystemIoError_Success) {
            error("could not read '%s': system error %u", path, io_res);
            AstContext_delete(ast);
            return 1;
        }

        switch (action) {
        case DriverAction_DumpLex:
            res = dump_lex_action(ast, source);
            break;
        case DriverAction_CheckLex:
            res = check_lex_action(ast, source);
            break;
        case DriverAction_CheckLexInvalid:
            res = check_lex_invalid_action(ast, source);
            break;
        case DriverAction_DumpParse:
            res = dump_parse_action(ast, source);
            break;
        case DriverAction_CheckParse:
            res = check_parse_action(ast, source);
            break;
        case DriverAction_CheckParseInvalid:
            res = check_parse_invalid_action(ast, source);
            break;
        case DriverAction_CheckTypes:
            res = check_types_action(ast, source);
            break;
        case DriverAction_CheckTypesInvalid:
            res = check_types_invalid_action(ast, source);
            break;
        case DriverAction_Unknown:
            assert(0 && "unreachable");
        }

        AstContext_delete(ast);
    }

    return res;
}
