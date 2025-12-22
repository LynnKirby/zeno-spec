#ifndef _ZENO_SPEC_SRC_EVAL_BYTECODE_H
#define _ZENO_SPEC_SRC_EVAL_BYTECODE_H

#include "src/support/stdint.h"

struct FunctionItem;
struct Writer;

#define OPCODE_LIST(X) \
    X(Trap)            \
    X(Return)          \
    X(PushInt32)

typedef enum Opcode {
    #define X(name) Opcode_##name,
    OPCODE_LIST(X)
    #undef X
    Opcode_COUNT ATTR_UNUSED
} Opcode;

typedef struct BytecodeFunction {
    struct FunctionItem const* item;
    uint8_t const* code;
    size_t code_size;
} BytecodeFunction;

/* Takes ownership of code data. */
BytecodeFunction* BytecodeFunction_new(
    struct FunctionItem const* item, uint8_t* code_data, size_t code_size
);
void BytecodeFunction_delete(BytecodeFunction* function);
void BytecodeFunction_dump(
    BytecodeFunction const* function, struct Writer* writer
);

#endif
