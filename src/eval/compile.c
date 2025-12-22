#include "src/eval/compile.h"
#include "src/ast/nodes.h"
#include "src/support/malloc.h"

#include <assert.h>

typedef struct CompileContext {
    uint8_t* data;
    size_t size;
    size_t capacity;
} CompileContext;

static void emit_u8(CompileContext* context, uint8_t value) {
    context->data = ensure_array_capacity(
        1, context->data, &context->size, &context->capacity, 1
    );
    context->data[context->size] = value;
    context->size += 1;
}

static void emit_u32(CompileContext* context, uint32_t value) {
    context->data = ensure_array_capacity(
        4, context->data, &context->size, &context->capacity, 1
    );
    context->data[context->size] = value;
    context->data[context->size + 1] = value >> 8;
    context->data[context->size + 2] = value >> 16;
    context->data[context->size + 3] = value >> 24;
    context->size += 4;
}

static void compile_expr(CompileContext* context, Expr const* expr) {
    switch (expr->kind) {
    case ExprKind_Return:
        compile_expr(context, ((ReturnExpr*)expr)->value);
        emit_u8(context, Opcode_Return);
        return;

    case ExprKind_IntLiteral:
        emit_u8(context, Opcode_PushInt32);
        emit_u32(context, BigInt_as_uint32(((IntLiteralExpr*)expr)->value));
        return;

    case ExprKind_SimpleType:
    case ExprKind_FunctionType:
        assert(0 && "runtime type values not supported yet");
        return;

    case ExprKind_Name:
        assert(0 && "variables not supported yet");
        return;
    }
}

BytecodeFunction* compile_function(FunctionItem const* function) {
    CompileContext context;

    context.data = NULL;
    context.size = 0;
    context.capacity = 0;

    compile_expr(&context, function->body);

    return BytecodeFunction_new(
        function,
        context.data,
        context.size
    );
}
