#include "src/lang/ast.h"

FunctionItem* FunctionItem_new(
    AstContext* context, StringRef name, Expr* return_type, Expr* body
) {
    FunctionItem* item;
    item = Arena_allocate(&context->arena, sizeof(FunctionItem));
    item->base.kind = ItemKind_Function;
    item->name = StringSet_add(&context->strings, name);
    item->body = body;
    item->return_type = return_type;
    item->decl = NULL;
    return item;
}

ReturnExpr* ReturnExpr_new(AstContext* context, Expr* value) {
    ReturnExpr* expr;
    expr = Arena_allocate(&context->arena, sizeof(ReturnExpr));
    expr->base.kind = ExprKind_Return;
    expr->value = value;
    return expr;
}

IntLiteralExpr* IntLiteralExpr_new(AstContext* context, BigInt value) {
    IntLiteralExpr* expr;
    expr = Arena_allocate(&context->arena, sizeof(IntLiteralExpr));
    expr->base.kind = ExprKind_IntLiteral;
    expr->value = value;
    return expr;
}

IdentifierExpr* IdentifierExpr_new(AstContext* context, StringRef value) {
    IdentifierExpr* expr;
    expr = Arena_allocate(&context->arena, sizeof(IdentifierExpr));
    expr->base.kind = ExprKind_Identifier;
    expr->value = StringSet_add(&context->strings, value);
    expr->referent = NULL;
    return expr;
}
