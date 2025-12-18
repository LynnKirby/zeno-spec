#include "src/driver/actions.h"
#include "src/ast/dump.h"
#include "src/driver/diagnostics.h"
#include "src/parsing/lex.h"
#include "src/parsing/parse.h"
#include "src/sema/type_checking.h"
#include "src/support/malloc.h"

static int is_type_check_action(DriverAction action) {
    switch (action) {
    case DriverAction_CheckTypes:
    case DriverAction_CheckTypesInvalid:
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
            FunctionItem_dump(
                (FunctionItem*)parse_result.u.item, Writer_stdout
            );
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
    if (res == 0 && is_type_check_action(action)) {
        TypeCheckResult type_check_result;
        type_check(&type_check_result, ast, (FunctionItem*)parse_result.u.item);

        switch (type_check_result.kind) {
        case TypeCheckResultKind_Success:
            if (action == DriverAction_CheckTypesInvalid) {
                res = 1;
            }
            break;
        case TypeCheckResultKind_UndeclaredName:
            if (action != DriverAction_CheckTypesInvalid) {
                write_undeclared_name_error(
                    Writer_stderr, path, &type_check_result.as.undeclared_name
                );
                res = 1;
            }
            break;
        case TypeCheckResultKind_ExpectedType:
            if (action != DriverAction_CheckTypesInvalid) {
                write_expected_type_error(
                    Writer_stderr, path, &type_check_result.as.expected_type
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

int check_types_action(AstContext* ast, SourceFile const* source) {
    return parse_action(ast, source, DriverAction_CheckTypes);
}

int check_types_invalid_action(AstContext* ast, SourceFile const* source) {
    return parse_action(ast, source, DriverAction_CheckTypesInvalid);
}
