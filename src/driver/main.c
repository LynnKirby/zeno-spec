#include "src/driver/commands.h"
#include "src/basic/diagnostic.h"
#include "src/driver/terminal_diagnostic_consumer.h"
#include "src/support/io.h"
#include "src/support/string_ref.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

typedef enum Command {
    Command_Tokenize,
    Command_Parse,
    Command_Check
} Command;

static StringRef get_program_name(int argc, char const* const* argv) {
    StringRef progname = STATIC_STRING_REF("zeno-spec");

    if (argc > 0) {
        char* slash_pos;
        slash_pos = strrchr(argv[0], '/');
        if (slash_pos != NULL) {
            /* TODO: make sure this is actually utf8 */
            StringRef from_argv0;
            from_argv0 = StringRef_from_zstr(slash_pos + 1);
            if (from_argv0.size > 0) {
                return from_argv0;
            }
        }
    }

    return progname;
}

static int get_command(
    Command* command, StringRef progname, int argc, char const* const* argv
) {
    if (argc > 1) {
        StringRef arg;
        arg = StringRef_from_zstr(argv[1]);
        if (StringRef_equal_zstr(arg, "tokenize")) {
            *command = Command_Tokenize;
            return 0;
        } else if (StringRef_equal_zstr(arg, "parse")) {
            *command = Command_Parse;
            return 0;
        } else if (StringRef_equal_zstr(arg, "check")) {
            *command = Command_Check;
            return 0;
        } else {
            Writer_write_str(Writer_stderr, progname);
            Writer_format(
                Writer_stderr, ": error: `%s` is not a command\n", arg.data
            );
        }
    } else {
        Writer_write_str(Writer_stderr, progname);
        Writer_format(Writer_stderr, ": error: command must be specified\n");
    }

    return 1;
}

int main(int argc, char const* const* argv) {
    int res;
    StringRef program_name;
    Command command;
    TerminalDiagnosticConsumer diagnostic_consumer;
    DiagnosticEngine* diagnostics;

    program_name = get_program_name(argc, argv);

    TerminalDiagnosticConsumer_init(
        &diagnostic_consumer, Writer_stderr, program_name
    );
    diagnostics = DiagnosticEngine_new(&diagnostic_consumer.base);

    res = get_command(&command, program_name, argc, argv);

    if (res == 0) {
        switch (command) {
        case Command_Tokenize:
            tokenize_command(diagnostics, argc - 2, argv + 2);
            break;

        case Command_Parse:
            parse_command(diagnostics, argc - 2, argv + 2);
            break;

        case Command_Check:
            check_command(diagnostics, argc - 2, argv + 2);
            break;
        }

        res = DiagnosticEngine_has_errors(diagnostics) ? 1 : 0;
    }

    DiagnosticEngine_delete(diagnostics);
    TerminalDiagnosticConsumer_destroy(&diagnostic_consumer);

    return res;
}
