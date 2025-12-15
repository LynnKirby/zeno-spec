#include "src/driver/actions.h"
#include "src/driver/diagnostics.h"
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

int main(int argc, char const* const* argv) {
    int res;
    char const* path = NULL;
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
            if (strcmp(arg, "--dump-binding") == 0) {
                assert(0 && "not implemented");
            }
            if (strcmp(arg, "--check-binding") == 0) {
                action = DriverAction_CheckBinding;
                continue;
            }
            if (strcmp(arg, "--check-binding-invalid") == 0) {
                action = DriverAction_CheckBindingInvalid;
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

    {
        ByteStringRef file_ref;
        ByteStringRef source;
        SystemIoError io_error;
        void* file_data;
        size_t file_size;

        io_error = SystemFile_read_all(file, &file_data, &file_size);

        if (io_error != SystemIoError_Success) {
            error("could not read '%s': system error %u", path, io_error);
            return 1;
        }

        source.data = file_data;
        source.size = file_size + 1;

        file_ref.data = path;
        file_ref.size = strlen(path);

        switch (action) {
        case DriverAction_DumpLex:
            res = dump_lex_action(file_ref, source);
            break;
        case DriverAction_CheckLex:
            res = check_lex_action(file_ref, source);
            break;
        case DriverAction_CheckLexInvalid:
            res = check_lex_invalid_action(file_ref, source);
            break;
        case DriverAction_DumpParse:
            res = dump_parse_action(file_ref, source);
            break;
        case DriverAction_CheckParse:
            res = check_parse_action(file_ref, source);
            break;
        case DriverAction_CheckParseInvalid:
            res = check_parse_invalid_action(file_ref, source);
            break;
        case DriverAction_CheckBinding:
            res = check_binding_action(file_ref, source);
            break;
        case DriverAction_CheckBindingInvalid:
            res = check_binding_invalid_action(file_ref, source);
            break;
        case DriverAction_Unknown:
            assert(0 && "unreachable");
        }

        xfree(file_data);
    }

    return res;
}
