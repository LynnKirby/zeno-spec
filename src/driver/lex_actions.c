#include "src/driver/actions.h"
#include "src/driver/diagnostics.h"
#include "src/lang/lex.h"
#include "src/support/io.h"
#include "src/support/malloc.h"

#include <assert.h>

static void dump_token_list(TokenList tokens) {
    size_t i;
    for (i = 0; i < tokens.size; i += 1) {
        Token_dump(&tokens.data[i], Writer_stdout);
    }
}

static int lex_action(
    AstContext* ast, SourceFile const* source, DriverAction action
) {
    int lex_okay = true;
    LexResult lex_result;

    lex_bytes(&lex_result, source, NULL);

    if (lex_result.is_tokens) {
        if (action == DriverAction_DumpLex) {
            dump_token_list(lex_result.u.tokens);
        }
        xfree(lex_result.u.tokens.data);
    } else {
        if (action != DriverAction_CheckLexInvalid) {
            write_lex_error(
                Writer_stderr, source->path.value, &lex_result.u.error
            );
        }
        lex_okay = false;
    }

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

int dump_lex_action(AstContext* ast, SourceFile const* source) {
    return lex_action(ast, source, DriverAction_DumpLex);
}

int check_lex_action(AstContext* ast, SourceFile const* source) {
    return lex_action(ast, source, DriverAction_CheckLex);
}

int check_lex_invalid_action(AstContext* ast, SourceFile const* source) {
    return lex_action(ast, source, DriverAction_CheckLexInvalid);
}
