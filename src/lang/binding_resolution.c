#include "src/lang/binding.h"
#include "src/support/hash_map.h"
#include "src/support/malloc.h"

#include <assert.h>
#include <setjmp.h>

typedef struct SymbolEnvNode {
    HashMap map;
    struct SymbolEnvNode* next;
} SymbolEnvNode;

typedef struct SymbolEnv {
    SymbolEnvNode* top;
} SymbolEnv;

static HashMapConfig const map_config = HASH_MAP_CONFIG(
    AstString,
    Decl const*,
    AstString_hash_generic,
    AstString_equal_generic
);

static void SymbolEnv_push_scope(SymbolEnv* env) {
    SymbolEnvNode* new_top;
    new_top = xmalloc(sizeof(SymbolEnvNode));
    HashMap_init(&new_top->map, &map_config);
    new_top->next = env->top;
    env->top = new_top;
}

static void SymbolEnv_pop_scope(SymbolEnv* env) {
    SymbolEnvNode* old_top;
    assert(env->top != NULL);
    old_top = env->top;
    env->top = old_top->next;
    HashMap_destroy(&old_top->map);
    xfree(old_top); /* TODO */
}

static void SymbolEnv_init(SymbolEnv* env) {
    env->top = NULL;
    SymbolEnv_push_scope(env);
}

static void SymbolEnv_destroy(SymbolEnv* env) {
    SymbolEnvNode* node;

    node = env->top;

    while (node != NULL) {
        SymbolEnvNode* next;
        next = node->next;
        HashMap_destroy(&node->map);
        xfree(node);
        node = next;
    }
}

static void SymbolEnv_add(
    SymbolEnv* env, AstString name, Decl const* decl
) {
    HashMap_set(&env->top->map, &map_config, &name, &decl);
}

Decl const* SymResEnv_get(SymbolEnv const* env, AstString name) {
    SymbolEnvNode* node;
    node = env->top;

    while (node != NULL) {
        Decl const* const* p_value;
        p_value = HashMap_get_value_by_key(&node->map, &map_config, &name);
        if (p_value != NULL) {
            return *p_value;
        }
        node = node->next;
    }

    return NULL;
}

typedef struct SymResContext {
    AstContext* ast;
    SymbolEnv env;
    SymbolBindingResult* result;
    jmp_buf exit_jmp_buf;
} SymResContext;

static void exit_symres(SymResContext* context) {
    longjmp(context->exit_jmp_buf, 1);
}

static void SymResExpr(SymResContext* context, Expr* expr);

static void SymResExpr_Identifier(
    SymResContext* context, IdentifierExpr* expr
) {
    UndefinedIdentifier* error;
    Decl const* decl;

    assert(expr->referent == NULL);
    decl = SymResEnv_get(&context->env, expr->value);

    if (decl != NULL) {
        expr->referent = decl;
        return;
    }

    error = &context->result->u.undefined_identifier;
    error->value = expr->value.value;
    error->pos.line = 0;
    error->pos.column = 0;

    context->result->kind = SymbolBindingResultKind_UndefinedIdentifier;
    exit_symres(context);
}

static void SymResExpr(SymResContext* context, Expr* expr) {
    switch (expr->kind) {
    /* SymResExpr-Return */
    case ExprKind_Return:
        SymResExpr(context, ((ReturnExpr*)expr)->value);
        break;

    /* SymResExpr-Literal */
    case ExprKind_IntLiteral:
        break;

    case ExprKind_Identifier:
        SymResExpr_Identifier(context, (IdentifierExpr*)expr);
        break;
    }
}

static void SymResItem_Function(
    SymResContext* context, FunctionItem* item
) {
    SymbolEnv_push_scope(&context->env);

    SymbolEnv_add(&context->env, item->name, item->decl);

    SymResExpr(context, item->return_type);
    SymResExpr(context, item->body);

    SymbolEnv_pop_scope(&context->env);
}

void symbol_resolution(
    SymbolBindingResult* result, AstContext* ast, FunctionItem* root
) {
    SymResContext context;

    Decl int32_decl;
    AstString int32_name;
    static StringRef int32_ref = STATIC_STRING_REF("Int32");

    context.ast = ast;
    context.result = result;
    SymbolEnv_init(&context.env);
    result->kind = SymbolBindingResultKind_Success;

    int32_name = AstContext_add_string(ast, int32_ref);
    int32_decl.kind = DeclKind_BuiltinInt;
    SymbolEnv_add(&context.env, int32_name, &int32_decl);

    if (setjmp(context.exit_jmp_buf) == 0) {
        SymResItem_Function(&context, root);
    }

    SymbolEnv_destroy(&context.env);
}
