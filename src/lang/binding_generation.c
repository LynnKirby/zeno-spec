#include "src/lang/binding.h"

#include <assert.h>

static void SymGen_Expr(AstContext* context, Expr* expr);

static void SymGen_Expr(AstContext* context, Expr* expr) {
    switch (expr->kind) {
    /* SymGenExpr-Return */
    case ExprKind_Return:
        SymGen_Expr(context, ((ReturnExpr*)expr)->value);
        break;

    /* SymGenExpr-Literal */
    case ExprKind_IntLiteral:
        break;

    /* SymGenExpr-Identifier */
    case ExprKind_Identifier:
        break;
    }
}

static void SymGenItem_Function(AstContext* context, FunctionItem* item) {
    Decl* decl;

    assert(item->decl == NULL);

    decl = Arena_allocate(&context->arena, sizeof(Decl));
    decl->kind = DeclKind_FunctionItem;
    item->decl = decl;

    SymGen_Expr(context, item->body);
}

void symbol_generation(AstContext* context, FunctionItem* root) {
    SymGenItem_Function(context, root);
}
