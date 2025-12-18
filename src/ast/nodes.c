#include "src/ast/nodes.h"
#include "src/ast/context.h"

#include <assert.h>

/*
 * Items
 */

FunctionItem* FunctionItem_new(
    AstContext* ast,
    AstString name,
    FunctionTypeExpr* type,
    Expr* body
) {
    FunctionItem* item;
    item = AstContext_allocate(ast, sizeof(FunctionItem));
    item->name = name;
    item->body = body;
    item->type = type;
    return item;
}

/*
 * Expressions
 */

IntLiteralExpr* IntLiteralExpr_new(AstContext* ast, BigInt value) {
    IntLiteralExpr* expr;
    expr = AstContext_allocate(ast, sizeof(IntLiteralExpr));
    expr->base.kind = ExprKind_IntLiteral;
    expr->value = value;
    return expr;
}

ReturnExpr* ReturnExpr_new(AstContext* ast, Expr* value) {
    ReturnExpr* expr;
    expr = AstContext_allocate(ast, sizeof(ReturnExpr));
    expr->base.kind = ExprKind_Return;
    expr->value = value;
    return expr;
}

NameExpr* NameExpr_new(AstContext* ast, AstString name) {
    NameExpr* expr;
    expr = AstContext_allocate(ast, sizeof(NameExpr));
    expr->base.kind = ExprKind_Name;
    expr->name = name;
    return expr;
}

SimpleTypeExpr* SimpleTypeExpr_new(struct AstContext* ast, SimpleTypeKind kind) {
    SimpleTypeExpr* expr;
    expr = AstContext_allocate(ast, sizeof(SimpleTypeExpr));
    expr->base.kind = ExprKind_SimpleType;
    expr->kind = kind;
    return expr;
}

FunctionTypeExpr* FunctionTypeExpr_new(
    struct AstContext* ast, Expr* return_type
) {
    FunctionTypeExpr* expr;
    expr = AstContext_allocate(ast, sizeof(FunctionTypeExpr));
    expr->base.kind = ExprKind_FunctionType;
    expr->return_type = return_type;
    expr->const_eval = NULL;
    expr->as_type = NULL;
    return expr;
}

/*
 * Types
 */

int Type_equal(Type const* left, Type const* right) {
    if (left == right) {
        return true;
    }

    if (left->kind != right->kind) {
        return false;
    }

    switch (left->kind) {
    case TypeKind_Simple:
        return ((SimpleType const*)left)->kind
            == ((SimpleType const*)right)->kind;

    case TypeKind_Function: {
        FunctionType const* left_func;
        FunctionType const* right_func;
        left_func = (FunctionType const*)left;
        right_func = (FunctionType const*)right;

        return Type_equal(left_func->return_type, right_func->return_type);
    }
    }

    assert(0 && "unreachable");
}

StringRef Type_name(Type const* type) {
    /* FIXME: full name should be cached in the type */
    switch (type->kind) {
    case TypeKind_Simple:
        switch (((SimpleType const*)type)->kind) {
        case SimpleTypeKind_Never:
            return StringRef_from_zstr("Never");
        case SimpleTypeKind_Void:
            return StringRef_from_zstr("Void");
        case SimpleTypeKind_Int32:
            return StringRef_from_zstr("Int32");
        case SimpleTypeKind_Type:
            return StringRef_from_zstr("Type");
        }
        break;

    case TypeKind_Function:
        /* FIXME */
        return StringRef_from_zstr("<function>");
    }

    assert(0 && "unreachable");
}

SimpleType* SimpleType_new(AstContext* ast, SimpleTypeKind kind) {
    SimpleType* type;
    type = AstContext_allocate(ast, sizeof(SimpleType));
    type->base.kind = TypeKind_Simple;
    type->kind = kind;
    return type;
}

FunctionType* FunctionType_new(AstContext* ast, Type* return_type) {
    FunctionType* type;
    type = AstContext_allocate(ast, sizeof(FunctionType));
    type->base.kind = TypeKind_Function;
    type->return_type = return_type;
    return type;
}
