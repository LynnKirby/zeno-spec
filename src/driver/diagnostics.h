#ifndef _ZENO_SPEC_SRC_DRIVER_DIAGNOSTICS_H
#define _ZENO_SPEC_SRC_DRIVER_DIAGNOSTICS_H

#include "src/parsing/parse.h"
#include "src/parsing/token.h"
#include "src/basic/diagnostic.h"

struct UndeclaredName;
struct ExpectedType;

/** Write error message from LexError. */
void write_lex_error(
    DiagnosticEngine* diagnostics, StringRef path, LexError const* error
);

/** Write error message from ParseError. */
void report_parse_error(
    DiagnosticEngine* diagnostics, StringRef path, ParseError const* error
);

/** Write error message returned by yacc. */
void report_yacc_error(DiagnosticEngine* diagnostics, ByteStringRef message);

/** Report undefined identifier. */
void report_undeclared_name(
    DiagnosticEngine* diagnostics, StringRef path, struct UndeclaredName const* error
);

/** Report mismatched type. */
void report_type_mismatch(
    DiagnosticEngine* diagnostics, StringRef path, struct ExpectedType const* error
);

#endif
