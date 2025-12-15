#ifndef _ZENOC_SRC_LANG_BINDING_H
#define _ZENOC_SRC_LANG_BINDING_H

#include "src/ast/context.h"
#include "src/ast/nodes.h"
#include "src/support/string_ref.h"
#include "src/support/source_pos.h"

typedef enum SymbolBindingResultKind {
    SymbolBindingResultKind_Success,
    SymbolBindingResultKind_UndefinedIdentifier
} SymbolBindingResultKind;

typedef struct UndefinedIdentifier {
    StringRef value;
    SourcePos pos;
} UndefinedIdentifier;

typedef struct SymbolBindingResult {
    SymbolBindingResultKind kind;
    union {
        UndefinedIdentifier undefined_identifier;
    } u;
} SymbolBindingResult;

/** Perform the generation phase of symbol binding. */
void symbol_generation(AstContext* context, FunctionItem* root);

/** Perform the resolution phase of symbol binding. */
void symbol_resolution(
    SymbolBindingResult* result, AstContext* context, FunctionItem* root
);

/** Perform symbol binding. */
static inline void symbol_binding(
    SymbolBindingResult* result, AstContext* context, FunctionItem* root
) {
    symbol_generation(context, root);
    symbol_resolution(result, context, root);
}

#endif
