#include "src/basic/diagnostic.h"
#include "src/support/malloc.h"
#include "src/support/array_writer.h"

#include <assert.h>

struct DiagnosticBuilder {
    DiagnosticEngine* engine;
    ArrayWriter writer;
    Diagnostic diagnostic;
};

struct DiagnosticEngine {
    DiagnosticConsumer* consumer;
    DiagnosticBuilder builder;
    unsigned active_builder : 1;
    unsigned has_errors : 1;
};

DiagnosticEngine* DiagnosticEngine_new(DiagnosticConsumer* consumer) {
    DiagnosticEngine* engine;
    engine = xmalloc(sizeof(DiagnosticEngine));
    engine->consumer = consumer;
    engine->has_errors = false;
    engine->active_builder = false;
    ArrayWriter_init(&engine->builder.writer);
    engine->builder.engine = engine;
    return engine;
}

void DiagnosticEngine_delete(DiagnosticEngine* engine) {
    assert(!engine->active_builder);
    ArrayWriter_destroy(&engine->builder.writer);
    xfree(engine);
}

int DiagnosticEngine_has_errors(DiagnosticEngine* engine) {
    return engine->has_errors;
}

DiagnosticBuilder* DiagnosticEngine_start_diagnostic(DiagnosticEngine* engine) {
    Diagnostic* diagnostic;

    assert(!engine->active_builder);
    engine->active_builder = true;

    ArrayWriter_reset(&engine->builder.writer);
    diagnostic = &engine->builder.diagnostic;

    diagnostic->category = 0;
    diagnostic->level = 0;
    diagnostic->source_name.size = 0;
    diagnostic->source_pos.line = 0;
    diagnostic->source_pos.column = 0;

    return &engine->builder;
}

void DiagnosticBuilder_emit(DiagnosticBuilder* builder) {
    Diagnostic* diagnostic;

    assert(builder->engine->active_builder);
    builder->engine->active_builder = false;

    diagnostic = &builder->diagnostic;

    assert(diagnostic->category != 0);
    assert(diagnostic->level != 0);

    diagnostic->message.data = builder->writer.data;
    diagnostic->message.size = builder->writer.size;

    builder->engine->consumer->handle(builder->engine->consumer, diagnostic);

    ArrayWriter_reset(&builder->writer);

    builder->engine->has_errors = diagnostic->level == DiagnosticLevel_Error;
}

void DiagnosticBuilder_set_source(
    DiagnosticBuilder* builder, StringRef source_name
) {
    builder->diagnostic.source_name = source_name;
}

void DiagnosticBuilder_set_line(DiagnosticBuilder* builder, uint32_t line) {
    builder->diagnostic.source_pos.line = line;
}

void DiagnosticBuilder_set_column(DiagnosticBuilder* builder, uint32_t column) {
    builder->diagnostic.source_pos.column = column;
}

void DiagnosticBuilder_set_pos(DiagnosticBuilder* builder, SourcePos pos) {
    builder->diagnostic.source_pos = pos;
}

void DiagnosticBuilder_set_level(
    DiagnosticBuilder* builder, DiagnosticLevel level
) {
    builder->diagnostic.level = level;
}

void DiagnosticBuilder_set_category(
    DiagnosticBuilder* builder, DiagnosticCategory category
) {
    builder->diagnostic.category = category;
}

Writer* DiagnosticBuilder_get_writer(DiagnosticBuilder* builder) {
    return &builder->writer.base;
}

