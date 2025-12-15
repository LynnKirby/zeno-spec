#include "src/lang/binding.h"

#include <assert.h>

static void SymGen_Expr(AstContext* ast, Expr* expr);

static void SymGen_Expr(AstContext* ast, Expr* expr) {
    switch (expr->kind) {
    /* SymGenExpr-Return */
    case ExprKind_Return:
        SymGen_Expr(ast, ((ReturnExpr*)expr)->value);
        break;

    /* SymGenExpr-Literal */
    case ExprKind_IntLiteral:
        break;

    /* SymGenExpr-Identifier */
    case ExprKind_Identifier:
        break;
    }
}

static void SymGenItem_Function(AstContext* ast, FunctionItem* item) {
    Decl* decl;

    assert(item->decl == NULL);

    decl = AstContext_allocate(ast, sizeof(Decl));
    decl->kind = DeclKind_FunctionItem;
    item->decl = decl;

    SymGen_Expr(ast, item->body);
}

void symbol_generation(AstContext* ast, FunctionItem* root) {
    SymGenItem_Function(ast, root);
}
