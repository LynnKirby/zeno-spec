#ifndef _ZENO_SPEC_SRC_AST_H
#define _ZENO_SPEC_SRC_AST_H

#include "src/base.h"
#include "src/io.h"

typedef struct AstContext {
    Arena arena;
} AstContext;

void AstContext_init(AstContext* context);
void AstContext_destroy(AstContext* context);

StringRef AstContext_intern_string(AstContext* context, StringRef str);

#define ITEM_KIND_LIST(X) \
    X(Function)

#define EXPR_KIND_LIST(X) \
    X(Item)               \
    X(IntLiteral)         \
    X(Return)

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

struct FunctionItem {
    Item base;
    StringRef name;
    Expr* body;
};

struct ItemExpr {
    Expr base;
    Item* item;
};

struct ReturnExpr {
    Expr base;
    Expr* value;
};

struct IntLiteralExpr {
    Expr base;
    uint32_t value;
};

FunctionItem* FunctionItem_new(AstContext* context, StringRef name, Expr* body);

ItemExpr* ItemExpr_new(AstContext* context, Item* item);
ReturnExpr* ReturnExpr_new(AstContext* context, Expr* value);
IntLiteralExpr* IntLiteralExpr_new(AstContext* context, uint32_t value);

void Item_dump(Item const* item, Writer* writer);
void Expr_dump(Expr const* expr, Writer* writer);

#define X(name) void name##Item_dump(name##Item const* item, Writer* writer);
ITEM_KIND_LIST(X)
#undef X

#define X(name) void name##Expr_dump(name##Expr const* expr, Writer* writer);
EXPR_KIND_LIST(X)
#undef X

#endif
