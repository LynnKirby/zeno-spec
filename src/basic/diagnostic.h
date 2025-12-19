#ifndef _ZENO_SPEC_SRC_BASIC_DIAGNOSTIC_H
#define _ZENO_SPEC_SRC_BASIC_DIAGNOSTIC_H

#include "src/support/source_pos.h"
#include "src/support/stdint.h"
#include "src/support/string_ref.h"
#include "src/support/io.h"

typedef enum DiagnosticLevel {
    DiagnosticLevel_Ignore = 1,
    DiagnosticLevel_Info,
    DiagnosticLevel_Warning,
    DiagnosticLevel_Error
} DiagnosticLevel;

typedef enum DiagnosticCategory {
    DiagnosticCategory_Driver = 1,
    DiagnosticCategory_System,
    DiagnosticCategory_Tokenize,
    DiagnosticCategory_Parse,
    DiagnosticCategory_TypeCheck
} DiagnosticCategory;

/** Fully resolved diagnostic. */
typedef struct Diagnostic {
    DiagnosticLevel level;
    DiagnosticCategory category;
    StringRef source_name;
    SourcePos source_pos;
    StringRef message;
} Diagnostic;

/** Diagnostic consumer interface. */
typedef struct DiagnosticConsumer {
    void (*handle)(
        struct DiagnosticConsumer* consumer,
        Diagnostic const* diagnostic
    );
} DiagnosticConsumer;

/** Incrementally builds a diagnostic. */
typedef struct DiagnosticBuilder DiagnosticBuilder;

/** Manages diagnostic emission. */
typedef struct DiagnosticEngine DiagnosticEngine;

DiagnosticEngine* DiagnosticEngine_new(DiagnosticConsumer* consumer);
void DiagnosticEngine_delete(DiagnosticEngine* engine);

int DiagnosticEngine_has_errors(DiagnosticEngine* engine);

/** Start a new diagnostic. The previous one must be emitted. */
DiagnosticBuilder* DiagnosticEngine_start_diagnostic(DiagnosticEngine* engine);

/** Finalize and emit diagnostic. */
void DiagnosticBuilder_emit(DiagnosticBuilder* builder);

void DiagnosticBuilder_set_source(
    DiagnosticBuilder* builder, StringRef source_name
);
void DiagnosticBuilder_set_line(DiagnosticBuilder* builder, uint32_t line);
void DiagnosticBuilder_set_column(DiagnosticBuilder* builder, uint32_t column);
void DiagnosticBuilder_set_pos(DiagnosticBuilder* builder, SourcePos pos);

/** Set minimum diagnostic level. */
void DiagnosticBuilder_set_level(
    DiagnosticBuilder* builder, DiagnosticLevel level
);

/** Set diagnostic category. */
void DiagnosticBuilder_set_category(
    DiagnosticBuilder* builder, DiagnosticCategory category
);

/** Get message writer. */
Writer* DiagnosticBuilder_get_writer(DiagnosticBuilder* builder);

#endif
