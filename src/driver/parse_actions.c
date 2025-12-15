#include "src/driver/actions.h"
#include "src/ast/dump.h"
#include "src/driver/diagnostics.h"
#include "src/lang/binding.h"
#include "src/lang/lex.h"
#include "src/lang/parse.h"
#include "src/support/malloc.h"

static int is_binding_action(DriverAction action) {
    switch (action) {
    case DriverAction_CheckBinding:
    case DriverAction_CheckBindingInvalid:
        return true;
    default:
        return false;
    }
}

static int parse_action(
    ByteStringRef path, ByteStringRef source, DriverAction action
) {
    int res = 0;
    AstContext* context;
    LexResult lex_result;
    ParseResult parse_result;

    lex_bytes(&lex_result, source, NULL);

    if (!lex_result.is_tokens) {
        write_lex_error(Writer_stderr, path, &lex_result.u.error);
        return 1;
    }

    context = AstContext_new();

    parse(&parse_result, context, &lex_result.u.tokens);
    xfree(lex_result.u.tokens.data);

    switch (parse_result.kind) {
    case ParseResultKind_Success:
        if (action == DriverAction_DumpParse) {
            Item_dump(parse_result.u.syntax, Writer_stdout);
        } else if (action == DriverAction_CheckParseInvalid) {
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

    /* FIXME: move this */
    if (res == 0 && is_binding_action(action)) {
        SymbolBindingResult binding_result;
        symbol_binding(
            &binding_result, context, (FunctionItem*)parse_result.u.syntax
        );

        switch (binding_result.kind) {
        case SymbolBindingResultKind_Success:
            if (action == DriverAction_CheckBindingInvalid) {
                res = 1;
            }
            break;
        case SymbolBindingResultKind_UndefinedIdentifier:
            if (action != DriverAction_CheckBindingInvalid) {
                write_undefined_identifier_error(
                    Writer_stderr, path, &binding_result.u.undefined_identifier
                );
                res = 1;
            }
            break;
        }
    }

    AstContext_delete(context);
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

int check_binding_action(ByteStringRef path, ByteStringRef source) {
    return parse_action(path, source, DriverAction_CheckBinding);
}

int check_binding_invalid_action(ByteStringRef path, ByteStringRef source) {
    return parse_action(path, source, DriverAction_CheckBindingInvalid);
}
