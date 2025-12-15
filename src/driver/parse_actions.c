#include "src/driver/actions.h"
#include "src/ast/dump.h"
#include "src/driver/diagnostics.h"
#include "src/lang/binding.h"
#include "src/syntax/lex.h"
#include "src/syntax/parse.h"
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
    AstContext* ast, SourceFile const* source, DriverAction action
) {
    int res = 0;
    LexResult lex_result;
    ParseResult parse_result;
    StringRef path;

    path = SourceFile_path(source).value;
    lex_source(&lex_result, ast, source, NULL);

    if (!lex_result.is_tokens) {
        write_lex_error(Writer_stderr, path, &lex_result.u.error);
        return 1;
    }

    parse(&parse_result, ast, &lex_result.u.tokens);
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
            &binding_result, ast, (FunctionItem*)parse_result.u.syntax
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

    return res;
}

int dump_parse_action(AstContext* ast, SourceFile const* source) {
    return parse_action(ast, source, DriverAction_DumpParse);
}

int check_parse_action(AstContext* ast, SourceFile const* source) {
    return parse_action(ast, source, DriverAction_CheckParse);
}

int check_parse_invalid_action(AstContext* ast, SourceFile const* source) {
    return parse_action(ast, source, DriverAction_CheckParseInvalid);
}

int check_binding_action(AstContext* ast, SourceFile const* source) {
    return parse_action(ast, source, DriverAction_CheckBinding);
}

int check_binding_invalid_action(AstContext* ast, SourceFile const* source) {
    return parse_action(ast, source, DriverAction_CheckBindingInvalid);
}
