#include "src/eval/bytecode.h"
#include "src/support/malloc.h"
#include "src/support/io.h"

BytecodeFunction* BytecodeFunction_new(
    struct FunctionItem const* item, uint8_t* code_data, size_t code_size
) {
    BytecodeFunction* func;
    func = xmalloc(sizeof(BytecodeFunction));
    func->item = item;
    func->code = code_data;
    func->code_size = code_size;
    return func;
}

void BytecodeFunction_delete(BytecodeFunction* function) {
    xfree((void*)function->code);
    xfree(function);
}

static void write_opcode(Writer* writer, Opcode opcode) {
    switch (opcode) {
    #define X(name)                           \
        case Opcode_##name:                   \
            Writer_write_zstr(writer, #name); \
            break;
    OPCODE_LIST(X)
    #undef X
    }
}

void BytecodeFunction_dump(BytecodeFunction const* function, Writer* writer) {
    size_t i;

    for (i = 0; i < function->code_size;) {
        write_opcode(writer, function->code[i]);

        switch (function->code[i]) {
        default:
            i += 1;
            break;

        case Opcode_PushInt32: {
            uint32_t value;
            value = function->code[1];
            value |= (uint32_t)function->code[2] << 8;
            value |= (uint32_t)function->code[3] << 16;
            value |= (uint32_t)function->code[4] << 24;
            Writer_write_zstr(writer, " 0x");
            Writer_write_uint(writer, value, 16);
            i += 5;
            break;
        }
        }

        Writer_write_zstr(writer, "\n");
    }
}
