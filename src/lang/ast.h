#ifndef _ZENO_SPEC_SRC_AST_H
#define _ZENO_SPEC_SRC_AST_H

#include "src/support/arena.h"
#include "src/support/bigint.h"
#include "src/support/io.h"
#include "src/support/string_ref.h"
#include "src/support/string_set.h"

typedef struct AstContext {
    Arena arena;
    StringSet strings;
} AstContext;

void AstContext_init(AstContext* context);
void AstContext_destroy(AstContext* context);

#define ITEM_KIND_LIST(X) \
    X(Function)

#define EXPR_KIND_LIST(X) \
    X(Identifier)         \
    X(IntLiteral)         \
    X(Return)

#define DECL_KIND_LIST(X) \
    X(BuiltinInt)         \
    X(FunctionItem)

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

typedef enum DeclKind {
    #define X(name) DeclKind_##name,
    DECL_KIND_LIST(X)
    #undef X
    DeclKind_COUNT ATTR_UNUSED
} DeclKind;

#define X(name) typedef struct name##Item name##Item;
ITEM_KIND_LIST(X)
#undef X

#define X(name) typedef struct name##Expr name##Expr;
EXPR_KIND_LIST(X)
#undef X

typedef struct Item {
    ItemKind kind;
} Item;

typedef struct Expr {
    ExprKind kind;
} Expr;

typedef struct Decl {
    DeclKind kind;
} Decl;

struct FunctionItem {
    Item base;
    StringSetItem name;
    Expr* body;
    Expr* return_type;
    Decl const* decl;
};

struct ReturnExpr {
    Expr base;
    Expr* value;
};

struct IntLiteralExpr {
    Expr base;
    BigInt value;
};

struct IdentifierExpr {
    Expr base;
    StringSetItem value;
    Decl const* referent;
};

FunctionItem* FunctionItem_new(
    AstContext* context,
    StringRef name,
    Expr* return_type,
    Expr* body
);

ReturnExpr* ReturnExpr_new(AstContext* context, Expr* value);
IntLiteralExpr* IntLiteralExpr_new(AstContext* context, BigInt value);
IdentifierExpr* IdentifierExpr_new(AstContext* context, StringRef value);

void Item_dump(Item const* item, Writer* writer);
void Expr_dump(Expr const* expr, Writer* writer);

#define X(name) void name##Item_dump(name##Item const* item, Writer* writer);
ITEM_KIND_LIST(X)
#undef X

#define X(name) void name##Expr_dump(name##Expr const* expr, Writer* writer);
EXPR_KIND_LIST(X)
#undef X

#endif
