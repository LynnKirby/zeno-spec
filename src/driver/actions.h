#ifndef _ZENO_SPEC_SRC_DRIVER_ACTIONS_H
#define _ZENO_SPEC_SRC_DRIVER_ACTIONS_H

#include "src/ast/context.h"
#include "src/ast/source.h"

typedef enum DriverAction {
    DriverAction_Unknown,
    DriverAction_DumpLex,
    DriverAction_CheckLex,
    DriverAction_CheckLexInvalid,
    DriverAction_DumpParse,
    DriverAction_CheckParse,
    DriverAction_CheckParseInvalid,
    DriverAction_CheckTypes,
    DriverAction_CheckTypesInvalid
} DriverAction;

int dump_lex_action(AstContext* ast, SourceFile const* source);
int check_lex_action(AstContext* ast, SourceFile const* source);
int check_lex_invalid_action(AstContext* ast, SourceFile const* source);

int dump_parse_action(AstContext* ast, SourceFile const* source);
int check_parse_action(AstContext* ast, SourceFile const* source);
int check_parse_invalid_action(AstContext* ast, SourceFile const* source);

int check_types_action(AstContext* ast, SourceFile const* source);
int check_types_invalid_action(AstContext* ast, SourceFile const* source);

#endif
