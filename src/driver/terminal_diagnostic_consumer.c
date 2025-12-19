#include "src/driver/terminal_diagnostic_consumer.h"

static void TerminalDiagnosticConsumer_handle(
    DiagnosticConsumer* base_consumer,
    Diagnostic const* diagnostic
) {
    TerminalDiagnosticConsumer* consumer;

    if (diagnostic->level == DiagnosticLevel_Ignore) {
        return;
    }

    consumer = (TerminalDiagnosticConsumer*)base_consumer;

    if (diagnostic->source_name.size == 0) {
        Writer_write_str(consumer->writer, consumer->program_name);
    } else {
        Writer_write_str(consumer->writer, diagnostic->source_name);
    }

    if (diagnostic->source_pos.line > 0) {
        Writer_write_zstr(consumer->writer, ":");
        Writer_write_uint(consumer->writer, diagnostic->source_pos.line, 10);

        if (diagnostic->source_pos.column > 0) {
            Writer_write_zstr(consumer->writer, ":");
            Writer_write_uint(
                consumer->writer, diagnostic->source_pos.column, 10
            );
        }
    }

    switch (diagnostic->level) {
    case DiagnosticLevel_Ignore:
    case DiagnosticLevel_Info:
        Writer_write_zstr(consumer->writer, ": info: ");
        break;

    case DiagnosticLevel_Warning:
        Writer_write_zstr(consumer->writer, ": warning: ");
        break;

    case DiagnosticLevel_Error:
        Writer_write_zstr(consumer->writer, ": error: ");
        break;
    }

    Writer_write_str(consumer->writer, diagnostic->message);
    Writer_write_zstr(consumer->writer, "\n");
}

void TerminalDiagnosticConsumer_init(
    TerminalDiagnosticConsumer* consumer, Writer* writer, StringRef program_name
) {
    consumer->base.handle = TerminalDiagnosticConsumer_handle;
    consumer->program_name = program_name;
    consumer->writer = writer;
}

void TerminalDiagnosticConsumer_destroy(TerminalDiagnosticConsumer* consumer) {
    (void)consumer;
    /* No-op */
}
