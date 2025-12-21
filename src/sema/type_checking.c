#include "src/sema/type_checking.h"
#include "src/sema/decl_map.h"

#include <assert.h>
#include <setjmp.h>

typedef struct TypeContext {
    DeclMap decls;
    Type* return_type; /* nullable */
    AstContext* ast;
    TypeCheckResult* result;
    jmp_buf exit_jmp_buf;
    Type* type_type;
    Type* never_type;
} TypeContext;

static void exit_type_checking(TypeContext* context) {
    longjmp(context->exit_jmp_buf, 1);
}

static void report_undeclared_name(TypeContext* context, NameExpr* expr) {
    context->result->kind = TypeCheckResultKind_UndeclaredName;
    context->result->as.undeclared_name.name = expr->name.value;
    context->result->as.undeclared_name.pos.line = 0;
    context->result->as.undeclared_name.pos.column = 0;
    exit_type_checking(context);
}

static void report_expected_type(
    TypeContext* context, Type* actual, Type* expected
) {
    context->result->kind = TypeCheckResultKind_ExpectedType;
    context->result->as.expected_type.actual = actual;
    context->result->as.expected_type.expected = expected;
    context->result->as.expected_type.pos.line = 0;
    context->result->as.expected_type.pos.column = 0;
    exit_type_checking(context);
}

static void add_simple_type(
    TypeContext* context, SimpleTypeKind kind, char const* name
) {
    Decl decl;

    decl.kind = DeclKind_Const;

    decl.as.const_decl.value =
        (Expr*)AstContext_simple_type_expr(context->ast, kind);
    decl.as.const_decl.type =
        (Type*)AstContext_simple_type(context->ast, SimpleTypeKind_Type);

    DeclMap_set(
        &context->decls,
        AstContext_add_string(context->ast, StringRef_from_zstr(name)),
        &decl
    );
}

static void add_prelude(TypeContext* context) {
    add_simple_type(context, SimpleTypeKind_Int32, "Int32");
}

/*
 * Type lifting - as_type
 */

static Type* as_type(TypeContext* context, Expr* expr);

static Type* as_type_impl(TypeContext* context, Expr* expr) {
    switch (expr->kind) {
    case ExprKind_SimpleType:
        return (Type*)AstContext_simple_type(
            context->ast, ((SimpleTypeExpr*)expr)->kind
        );

    case ExprKind_FunctionType: {
        FunctionTypeExpr* func_type_expr;
        Type* return_type;

        func_type_expr = (FunctionTypeExpr*)expr;

        return_type = as_type(context, func_type_expr->return_type);
        if (return_type == NULL) {
            return NULL;
        }

        return (Type*)FunctionType_new(context->ast, return_type);
    }

    default:
        return NULL;
    }
}

static Type* as_type(TypeContext* context, Expr* expr) {
    if (expr->as_type == NULL) {
        expr->as_type = as_type_impl(context, expr);
    }
    return expr->as_type;
}

/*
 * Constant evaluation
 */

static Expr* const_eval(TypeContext* context, Expr* expr);

static Expr* const_eval_impl(TypeContext* context, Expr* expr) {
    switch (expr->kind) {
    default:
        return NULL;

    /* ConstEval-Value */
    case ExprKind_IntLiteral:
    case ExprKind_SimpleType:
        return expr;

    /* ConstEval-Name */
    case ExprKind_Name: {
        NameExpr* name_expr;
        Decl const* decl;

        name_expr = (NameExpr*)expr;
        decl = DeclMap_get(&context->decls, name_expr->name);

        if (decl == NULL) {
            report_undeclared_name(context, name_expr);
        }

        switch (decl->kind) {
        case DeclKind_Const:
            return decl->as.const_decl.value;

        default:
            return NULL;
        }
    }

    /* ConstEval-FunctionType */
    case ExprKind_FunctionType: {
        FunctionTypeExpr* func_type_expr;
        Expr* return_type;

        func_type_expr = (FunctionTypeExpr*)expr;

        return_type = const_eval(context, func_type_expr->return_type);
        if (return_type == NULL) {
            return NULL;
        }

        return (Expr*)FunctionTypeExpr_new(context->ast, return_type);
    }
    }
}

static Expr* const_eval(TypeContext* context, Expr* expr) {
    if (expr->const_eval == NULL) {
        expr->const_eval = const_eval_impl(context, expr);
    }
    return expr->const_eval;
}

/*
 * Typing
 */

static void type_function_item(TypeContext* context, FunctionItem* item);

static Type* type_expr(TypeContext* context, Expr* expr);

static Type* type_expr_impl(TypeContext* context, Expr* expr) {
    switch (expr->kind) {
    /* TypeExpr-IntLiteral */
    case ExprKind_IntLiteral:
        return (Type*)AstContext_simple_type(
            context->ast, SimpleTypeKind_Int32
        );

    /* TypeExpr-Return */
    case ExprKind_Return: {
        ReturnExpr* return_expr;

        assert(context->return_type != NULL); /* TODO: report error */
        return_expr = (ReturnExpr*)expr;

        type_expr(context, return_expr->value);

        if (!Type_equal(context->return_type, return_expr->value->type)) {
            report_expected_type(
                context, return_expr->value->type, context->return_type
            );
        }

        return context->never_type;
    }

    case ExprKind_Name: {
        NameExpr* name_expr;
        Decl const* decl;

        name_expr = (NameExpr*)expr;
        decl = DeclMap_get(&context->decls, name_expr->name);

        if (decl == NULL) {
            report_undeclared_name(context, name_expr);
        }

        switch (decl->kind) {
        /* TypeExpr-ConstName */
        case DeclKind_Const:
            return decl->as.const_decl.type;
        }

        break;
    }

    /* TypeExpr-Type */
    case ExprKind_SimpleType:
    case ExprKind_FunctionType:
        return context->type_type;
    }

    assert(0 && "unreachable");
}

static Type* type_expr(TypeContext* context, Expr* expr) {
    if (expr->type == NULL) {
        expr->type = type_expr_impl(context, expr);
    }
    return expr->type;
}

static void type_function_item(TypeContext* context, FunctionItem* item) {
    Expr* func_type_expr;
    FunctionType* func_type;
    Type* old_return_type;

    func_type_expr = const_eval(context, (Expr*)item->type);
    type_expr(context, func_type_expr);

    if (!Type_equal(func_type_expr->type, context->type_type)) {
        report_expected_type(context, func_type_expr->type, context->type_type);
    }

    func_type = (FunctionType*)as_type(context, func_type_expr);
    assert(func_type->base.kind == TypeKind_Function);

    DeclMap_push_scope(&context->decls);
    old_return_type = context->return_type;
    context->return_type = func_type->return_type;

    type_expr(context, item->body);

    context->return_type = old_return_type;
    DeclMap_pop_scope(&context->decls);
}

void type_check(TypeCheckResult* result, AstContext* ast, FunctionItem* item) {
    TypeContext context;

    DeclMap_init(&context.decls);
    context.return_type = NULL;
    context.ast = ast;
    context.result = result;
    context.type_type = (Type*)AstContext_simple_type(ast, SimpleTypeKind_Type);
    context.never_type = (Type*)AstContext_simple_type(ast, SimpleTypeKind_Never);

    result->kind = TypeCheckResultKind_Success;

    add_prelude(&context);

    if (setjmp(context.exit_jmp_buf) == 0) {
        type_function_item(&context, item);
    }

    DeclMap_destroy(&context.decls);
}
