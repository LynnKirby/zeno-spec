#ifndef _ZENO_SPEC_SRC_DRIVER_DIAGNOSTICS_H
#define _ZENO_SPEC_SRC_DRIVER_DIAGNOSTICS_H

#include "src/parsing/parse.h"
#include "src/parsing/token.h"
#include "src/support/io.h"

struct UndeclaredName;
struct ExpectedType;

/** Write error message from LexError. */
void write_lex_error(
    Writer* writer, StringRef path, LexError const* error
);

/** Write error message from ParseError. */
void write_parse_error(
    Writer* writer, StringRef path, ParseError const* error
);

/** Write error message returned by yacc. */
void write_yacc_error(Writer* writer, ByteStringRef message);

/** Report undefined identifier. */
void write_undeclared_name_error(
    Writer* writer, StringRef path, struct UndeclaredName const* error
);

/** Report mismatched type. */
void write_expected_type_error(
    Writer* writer, StringRef path, struct ExpectedType const* error
);

#endif
