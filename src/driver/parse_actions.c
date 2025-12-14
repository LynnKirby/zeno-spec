#include "src/driver/actions.h"
#include "src/driver/diagnostics.h"
#include "src/lang/lex.h"
#include "src/lang/parse.h"

static int parse_action(
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
            write_lex_error(Writer_stderr, path, &parse_result.u.lex_error);
            res = 1;
        }
        break;
    case ParseResultKind_ParseError:
        if (action != DriverAction_CheckParseInvalid) {
            write_parse_error(Writer_stderr, path, &parse_result.u.parse_error);
            res = 1;
        }
        break;
    case ParseResultKind_YaccError:
        write_yacc_error(Writer_stderr, parse_result.u.yacc_error);
        res = 1;
        break;
    }

    Lexer_destroy(&lexer);
    AstContext_destroy(&context);
    return res;
}

int dump_parse_action(ByteStringRef path, ByteStringRef source) {
    return parse_action(path, source, DriverAction_DumpParse);
}

int check_parse_action(ByteStringRef path, ByteStringRef source) {
    return parse_action(path, source, DriverAction_CheckParse);
}

int check_parse_invalid_action(ByteStringRef path, ByteStringRef source) {
    return parse_action(path, source, DriverAction_CheckParseInvalid);
}
