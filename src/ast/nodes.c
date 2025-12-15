#include "src/ast/nodes.h"

FunctionItem* FunctionItem_new(
    AstContext* ast, AstString name, Expr* return_type, Expr* body
) {
    FunctionItem* item;
    item = AstContext_allocate(ast, sizeof(FunctionItem));
    item->base.kind = ItemKind_Function;
    item->name = name;
    item->body = body;
    item->return_type = return_type;
    item->decl = NULL;
    return item;
}

ReturnExpr* ReturnExpr_new(AstContext* ast, Expr* value) {
    ReturnExpr* expr;
    expr = AstContext_allocate(ast, sizeof(ReturnExpr));
    expr->base.kind = ExprKind_Return;
    expr->value = value;
    return expr;
}

IntLiteralExpr* IntLiteralExpr_new(AstContext* ast, BigInt value) {
    IntLiteralExpr* expr;
    expr = AstContext_allocate(ast, sizeof(IntLiteralExpr));
    expr->base.kind = ExprKind_IntLiteral;
    expr->value = value;
    return expr;
}

IdentifierExpr* IdentifierExpr_new(AstContext* ast, AstString value) {
    IdentifierExpr* expr;
    expr = AstContext_allocate(ast, sizeof(IdentifierExpr));
    expr->base.kind = ExprKind_Identifier;
    expr->value = value;
    expr->referent = NULL;
    return expr;
}
