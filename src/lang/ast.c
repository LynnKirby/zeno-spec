#include "src/lang/ast.h"

#include <string.h>

void AstContext_init(AstContext* context) {
    Arena_init(&context->arena);
}

void AstContext_destroy(AstContext* context) {
    Arena_destroy(&context->arena);
}

StringRef AstContext_intern_string(AstContext* context, StringRef str) {
    /* TODO: string table */
    StringRef copy;
    char* data;
    data = Arena_allocate(&context->arena, str.size);
    memcpy(data, str.data, str.size);
    copy.data = data;
    copy.size = str.size;
    return copy;
}

FunctionItem* FunctionItem_new(AstContext* context, StringRef name, Expr* body) {
    FunctionItem* item;
    item = Arena_allocate(&context->arena, sizeof(FunctionItem));
    item->base.kind = ItemKind_Function;
    item->name = AstContext_intern_string(context, name);
    item->body = body;
    return item;
}

ItemExpr* ItemExpr_new(AstContext* context, Item* item) {
    ItemExpr* expr;
    expr = Arena_allocate(&context->arena, sizeof(ItemExpr));
    expr->base.kind = ExprKind_Item;
    expr->item = item;
    return expr;
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

/*
 * Text dump.
 */

static void Item_dump_internal(Item const* item, Writer* writer, int indent);

void Item_dump(Item const* item, Writer* writer) {
    Item_dump_internal(item, writer, 0);
    Writer_print(writer, "\n");
}

static void Expr_dump_internal(Expr const* expr, Writer* writer, int indent);

void Expr_dump(Expr const* expr, Writer* writer) {
    Expr_dump_internal(expr, writer, 0);
    Writer_print(writer, "\n");
}

#define X(name)                                                    \
    static void name##Item_dump_internal(                          \
        name##Item const* item, Writer* writer, int indent         \
    );                                                             \
    void name##Item_dump(name##Item const* item, Writer* writer) { \
        name##Item_dump_internal(item, writer, 0);                 \
    }
ITEM_KIND_LIST(X)
#undef X

#define X(name)                                                    \
    static void name##Expr_dump_internal(                          \
        name##Expr const* expr, Writer* writer, int indent         \
    );                                                             \
    void name##Expr_dump(name##Expr const* expr, Writer* writer) { \
        name##Expr_dump_internal(expr, writer, 0);                 \
    }
EXPR_KIND_LIST(X)
#undef X

static void Item_dump_internal(Item const* item, Writer* writer, int indent) {
    switch (item->kind) {
    #define X(name)                                                            \
        case ItemKind_##name:                                                  \
            name##Item_dump_internal((name##Item const*)item, writer, indent); \
            break;
            ITEM_KIND_LIST(X)
    #undef X
    }
}

static void Expr_dump_internal(Expr const* expr, Writer* writer, int indent) {
    switch (expr->kind) {
    #define X(name)                                                            \
        case ExprKind_##name:                                                  \
            name##Expr_dump_internal((name##Expr const*)expr, writer, indent); \
            break;
            EXPR_KIND_LIST(X)
    #undef X
    }
}

static void write_indent(Writer* writer, int indent) {
    while (indent > 0) {
        Writer_write(writer, "  ", 2);
        indent -= 1;
    }
}

static void FunctionItem_dump_internal(
    FunctionItem const* item, Writer* writer, int indent
) {
    Writer_print(writer, "FunctionItem(\n");

    indent += 1;

    write_indent(writer, indent);
    Writer_print(writer, "name = \"");
    Writer_write_str(writer, item->name);
    Writer_print(writer, "\",\n");

    if (item->body != NULL) {
        write_indent(writer, indent);
        Writer_print(writer, "body = ");
        Expr_dump_internal(item->body, writer, indent);
        Writer_print(writer, ",\n");
    }

    indent -= 1;

    write_indent(writer, indent);
    Writer_print(writer, ")");
}

static void ItemExpr_dump_internal(
    ItemExpr const* item, Writer* writer, int indent
) {
    Item_dump_internal(item->item, writer, indent);
}

static void ReturnExpr_dump_internal(
    ReturnExpr const* item, Writer* writer, int indent
) {
    Writer_print(writer, "ReturnExpr(\n");

    indent += 1;

    write_indent(writer, indent);
    Writer_print(writer, "value = ");
    Expr_dump_internal(item->value, writer, indent);
    Writer_print(writer, ",\n");

    indent -= 1;

    write_indent(writer, indent);
    Writer_print(writer, ")");
}

static void IntLiteralExpr_dump_internal(
    IntLiteralExpr const* item, Writer* writer, int indent
) {
    (void)indent; /* unused */
    Writer_print(writer, "IntLiteralExpr(value = ");
    BigInt_write(writer, item->value, 10);
    Writer_print(writer, ")");
}
