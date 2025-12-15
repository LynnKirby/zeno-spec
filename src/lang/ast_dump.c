#include "src/lang/ast.h"

static void Item_dump_internal(Item const* item, Writer* writer, int indent);

void Item_dump(Item const* item, Writer* writer) {
    Item_dump_internal(item, writer, 0);
    Writer_format(writer, "\n");
}

static void Expr_dump_internal(Expr const* expr, Writer* writer, int indent);

void Expr_dump(Expr const* expr, Writer* writer) {
    Expr_dump_internal(expr, writer, 0);
    Writer_format(writer, "\n");
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
    Writer_format(writer, "FunctionItem(\n");

    indent += 1;

    write_indent(writer, indent);
    Writer_format(writer, "name = \"");
    Writer_write_str(writer, item->name.string);
    Writer_format(writer, "\",\n");

    if (item->body != NULL) {
        write_indent(writer, indent);
        Writer_format(writer, "body = ");
        Expr_dump_internal(item->body, writer, indent);
        Writer_format(writer, ",\n");
    }

    if (item->return_type != NULL) {
        write_indent(writer, indent);
        Writer_format(writer, "return_type = ");
        Expr_dump_internal(item->return_type, writer, indent);
        Writer_format(writer, ",\n");
    }

    indent -= 1;

    write_indent(writer, indent);
    Writer_format(writer, ")");
}

static void ReturnExpr_dump_internal(
    ReturnExpr const* item, Writer* writer, int indent
) {
    Writer_format(writer, "ReturnExpr(\n");

    indent += 1;

    write_indent(writer, indent);
    Writer_format(writer, "value = ");
    Expr_dump_internal(item->value, writer, indent);
    Writer_format(writer, ",\n");

    indent -= 1;

    write_indent(writer, indent);
    Writer_format(writer, ")");
}

static void IntLiteralExpr_dump_internal(
    IntLiteralExpr const* item, Writer* writer, int indent
) {
    (void)indent; /* unused */
    Writer_format(writer, "IntLiteralExpr(value = ");
    BigInt_write(writer, item->value, 10);
    Writer_format(writer, ")");
}

static void IdentifierExpr_dump_internal(
    IdentifierExpr const* expr, Writer* writer, int indent
) {
    (void)indent; /* unused */
    Writer_format(writer, "IdentifierExpr(value = \"");
    Writer_write_str(writer, expr->value.string);
    Writer_format(writer, "\")");
}
