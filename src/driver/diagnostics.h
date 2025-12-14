#ifndef _ZENO_SPEC_SRC_DRIVER_DIAGNOSTICS_H
#define _ZENO_SPEC_SRC_DRIVER_DIAGNOSTICS_H

#include "src/lang/ast.h"
#include "src/lang/parse.h"
#include "src/lang/token.h"
#include "src/support/io.h"

/** Write error message from LexError. */
void write_lex_error(
    Writer* writer, ByteStringRef path, LexError const* error
);

/** Write error message from ParseError. */
void write_parse_error(
    Writer* writer, ByteStringRef path, ParseError const* error
);

/** Write error message returned by yacc. */
void write_yacc_error(Writer* writer, ByteStringRef message);

#endif
