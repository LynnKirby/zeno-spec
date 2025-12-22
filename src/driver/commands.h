#ifndef _ZENO_SPEC_SRC_DRIVER_COMMANDS_H
#define _ZENO_SPEC_SRC_DRIVER_COMMANDS_H

#include "src/basic/diagnostic.h"
#include "src/support/string_ref.h"

void tokenize_command(
    DiagnosticEngine* diagnostics, int argc, char const* const* argv
);
void parse_command(
    DiagnosticEngine* diagnostics, int argc, char const* const* argv
);
void check_command(
    DiagnosticEngine* diagnostics, int argc, char const* const* argv
);
void compile_command(
    DiagnosticEngine* diagnostics, int argc, char const* const* argv
);

#endif
