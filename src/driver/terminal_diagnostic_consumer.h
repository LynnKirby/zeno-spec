#ifndef _ZENO_SPEC_SRC_DRIVER_TERMINAL_DIAGNOSTIC_CONSUMER_H
#define _ZENO_SPEC_SRC_DRIVER_TERMINAL_DIAGNOSTIC_CONSUMER_H

#include "src/basic/diagnostic.h"
#include "src/support/io.h"
#include "src/support/string_ref.h"

typedef struct TerminalDiagnosticConsumer {
    DiagnosticConsumer base;
    StringRef program_name;
    Writer* writer;
} TerminalDiagnosticConsumer;

void TerminalDiagnosticConsumer_init(
    TerminalDiagnosticConsumer* consumer, Writer* writer, StringRef program_name
);
void TerminalDiagnosticConsumer_destroy(TerminalDiagnosticConsumer* consumer);

#endif
