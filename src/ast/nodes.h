#ifndef _ZENO_SPEC_SRC_AST_NODES_H
#define _ZENO_SPEC_SRC_AST_NODES_H

#include "src/ast/string.h"
#include "src/support/bigint.h"

struct AstContext;

#define ITEM_KIND_LIST(X) \
    X(Function)

#define EXPR_KIND_LIST(X) \
    X(IntLiteral)         \
    X(Return)             \
    X(Name)               \
    X(SimpleType)         \
    X(FunctionType)

#define TYPE_KIND_LIST(X) \
    X(Simple)             \
    X(Function)

#define SIMPLE_TYPE_LIST(X) \
    X(Never)                \
    X(Void)                 \
    X(Int32)                \
    X(Type)

typedef enum ItemKind {
    #define X(name) ItemKind_##name,
    ITEM_KIND_LIST(X)
    #undef X
    ItemKind_COUNT ATTR_UNUSED
} ItemKind;

typedef enum ExprKind {
    #define X(name) ExprKind_##name,
    EXPR_KIND_LIST(X)
    #undef X
    ExprKind_COUNT ATTR_UNUSED
} ExprKind;

typedef enum TypeKind {
    #define X(name) TypeKind_##name,
    TYPE_KIND_LIST(X)
    #undef X
    TypeKind_COUNT ATTR_UNUSED
} TypeKind;

typedef enum SimpleTypeKind {
    #define X(name) SimpleTypeKind_##name,
    SIMPLE_TYPE_LIST(X)
    #undef X
    SimpleTypeKind_COUNT ATTR_UNUSED
} SimpleTypeKind;

#define X(name) typedef struct name##Item name##Item;
ITEM_KIND_LIST(X)
#undef X

#define X(name) typedef struct name##Expr name##Expr;
EXPR_KIND_LIST(X)
#undef X

#define X(name) typedef struct name##Type name##Type;
TYPE_KIND_LIST(X)
#undef X

typedef struct Item {
    ItemKind kind;
} Item;

typedef struct Expr {
    ExprKind kind;
} Expr;

typedef struct Type {
    TypeKind kind;
} Type;

/*
 * Items
 */

struct FunctionItem {
    Item base;
    AstString name;
    FunctionTypeExpr* type;
    Expr* body;
};

FunctionItem* FunctionItem_new(
    struct AstContext* ast,
    AstString name,
    FunctionTypeExpr* type,
    Expr* body
);

/*
 * Expressions
 */

struct IntLiteralExpr {
    Expr base;
    BigInt value;
};

IntLiteralExpr* IntLiteralExpr_new(struct AstContext* ast, BigInt value);

struct ReturnExpr {
    Expr base;
    Expr* value;
};

ReturnExpr* ReturnExpr_new(struct AstContext* ast, Expr* value);

struct NameExpr {
    Expr base;
    AstString name;
};

NameExpr* NameExpr_new(struct AstContext* ast, AstString name);

struct SimpleTypeExpr {
    Expr base;
    SimpleTypeKind kind;
};

SimpleTypeExpr* SimpleTypeExpr_new(struct AstContext* ast, SimpleTypeKind kind);

struct FunctionTypeExpr {
    Expr base;
    Expr* return_type;

    FunctionTypeExpr* const_eval; /* nullable */
    FunctionType* as_type; /* nullable */
};

FunctionTypeExpr* FunctionTypeExpr_new(
    struct AstContext* ast, Expr* return_type
);

/*
 * Types
 */

int Type_equal(Type const* left, Type const* right);

StringRef Type_name(Type const* type);

struct SimpleType {
    Type base;
    SimpleTypeKind kind;
};

SimpleType* SimpleType_new(struct AstContext* ast, SimpleTypeKind kind);

struct FunctionType {
    Type base;
    Type* return_type;
};

FunctionType* FunctionType_new(struct AstContext* ast, Type* return_type);

#endif
