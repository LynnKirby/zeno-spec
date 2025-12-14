#include "src/driver/actions.h"
#include "src/driver/diagnostics.h"
#include "src/lang/lex.h"
#include "src/support/io.h"

#include <assert.h>

static int lex_action(
    ByteStringRef path,
    ByteStringRef source,
    DriverAction action
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
                write_lex_error(Writer_stderr, path, &lex_result.u.error);
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

int dump_lex_action(ByteStringRef path, ByteStringRef source) {
    return lex_action(path, source, DriverAction_DumpLex);
}

int check_lex_action(ByteStringRef path, ByteStringRef source) {
    return lex_action(path, source, DriverAction_CheckLex);
}

int check_lex_invalid_action(ByteStringRef path, ByteStringRef source) {
    return lex_action(path, source, DriverAction_CheckLexInvalid);
}
