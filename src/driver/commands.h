#ifndef _ZENO_SPEC_SRC_DRIVER_COMMANDS_H
#define _ZENO_SPEC_SRC_DRIVER_COMMANDS_H

#include "src/support/string_ref.h"

int tokenize_command(StringRef progname, int argc, char const* const* argv);
int parse_command(StringRef progname, int argc, char const* const* argv);
int check_command(StringRef progname, int argc, char const* const* argv);

#endif
