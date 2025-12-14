#ifndef _ZENO_SPEC_SRC_DRIVER_ACTIONS_H
#define _ZENO_SPEC_SRC_DRIVER_ACTIONS_H

#include "src/support/string_ref.h"

typedef enum DriverAction {
    DriverAction_Unknown,
    DriverAction_DumpLex,
    DriverAction_CheckLex,
    DriverAction_CheckLexInvalid,
    DriverAction_DumpParse,
    DriverAction_CheckParse,
    DriverAction_CheckParseInvalid
} DriverAction;

int dump_lex_action(ByteStringRef path, ByteStringRef source);
int check_lex_action(ByteStringRef path, ByteStringRef source);
int check_lex_invalid_action(ByteStringRef path, ByteStringRef source);

int dump_parse_action(ByteStringRef path, ByteStringRef source);
int check_parse_action(ByteStringRef path, ByteStringRef source);
int check_parse_invalid_action(ByteStringRef path, ByteStringRef source);

#endif
