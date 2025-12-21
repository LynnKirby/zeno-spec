#include "src/ast/dump.h"

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

static void Type_dump_internal(Type const* type, Writer* writer, int indent);

void Type_dump(Type const* type, Writer* writer) {
    Type_dump_internal(type, writer, 0);
    Writer_format(writer, "\n");
}

#define X(name)                                                    \
    static void name##Item_dump_internal(                          \
        name##Item const* item, Writer* writer, int indent         \
    );                                                             \
    void name##Item_dump(name##Item const* item, Writer* writer) { \
        name##Item_dump_internal(item, writer, 0);                 \
        Writer_format(writer, "\n");                               \
    }
ITEM_KIND_LIST(X)
#undef X

#define X(name)                                                    \
    static void name##Expr_dump_internal(                          \
        name##Expr const* expr, Writer* writer, int indent         \
    );                                                             \
    void name##Expr_dump(name##Expr const* expr, Writer* writer) { \
        name##Expr_dump_internal(expr, writer, 0);                 \
        Writer_format(writer, "\n");                               \
    }
EXPR_KIND_LIST(X)
#undef X

#define X(name)                                                    \
    static void name##Type_dump_internal(                          \
        name##Type const* type, Writer* writer, int indent         \
    );                                                             \
    void name##Type_dump(name##Type const* type, Writer* writer) { \
        name##Type_dump_internal(type, writer, 0);                 \
        Writer_format(writer, "\n");                               \
    }
TYPE_KIND_LIST(X)
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
    if (expr->const_eval != NULL) {
        expr = expr->const_eval;
    }

    switch (expr->kind) {
    #define X(name)                                                            \
        case ExprKind_##name:                                                  \
            name##Expr_dump_internal((name##Expr const*)expr, writer, indent); \
            break;
            EXPR_KIND_LIST(X)
    #undef X
    }
}

static void Type_dump_internal(Type const* type, Writer* writer, int indent) {
    switch (type->kind) {
    #define X(name)                                                            \
        case TypeKind_##name:                                                  \
            name##Type_dump_internal((name##Type const*)type, writer, indent); \
            break;
            TYPE_KIND_LIST(X)
    #undef X
    }
}

static void write_indent(Writer* writer, int indent) {
    while (indent > 0) {
        Writer_write(writer, "  ", 2);
        indent -= 1;
    }
}

/*
 * Items
 */

static void FunctionItem_dump_internal(
    FunctionItem const* item, Writer* writer, int indent
) {
    Writer_format(writer, "FunctionItem(\n");
    indent += 1;

    write_indent(writer, indent);
    Writer_format(writer, "name = \"");
    Writer_write_str(writer, item->name.value);
    Writer_format(writer, "\",\n");

    write_indent(writer, indent);
    Writer_format(writer, "type = ");
    FunctionTypeExpr_dump_internal(item->type, writer, indent);
    Writer_format(writer, ",\n");

    write_indent(writer, indent);
    Writer_format(writer, "body = ");
    Expr_dump_internal(item->body, writer, indent);
    Writer_format(writer, ",\n");

    indent -= 1;
    write_indent(writer, indent);
    Writer_format(writer, ")");
}

/*
 * Expressions
 */

static void IntLiteralExpr_dump_internal(
    IntLiteralExpr const* expr, Writer* writer, int indent
) {
    Writer_format(writer, "IntLiteralExpr(value = ");
    BigInt_write(writer, expr->value, 10);

    if (expr->base.type != NULL) {
        Writer_format(writer, ", type = ");
        Type_dump_internal(expr->base.type, writer, indent);
    }

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

    if (item->base.type != NULL) {
        write_indent(writer, indent);
        Writer_format(writer, "type = ");
        Type_dump_internal(item->base.type, writer, indent);
        Writer_format(writer, ",\n");
    }

    indent -= 1;
    write_indent(writer, indent);
    Writer_format(writer, ")");
}

static void NameExpr_dump_internal(
    NameExpr const* expr, Writer* writer, int indent
) {
    (void)indent; /* unused */
    Writer_format(writer, "NameExpr(value = \"");
    Writer_write_str(writer, expr->name.value);
    Writer_format(writer, "\"");

    if (expr->base.type != NULL) {
        Writer_format(writer, ", type = ");
        Type_dump_internal(expr->base.type, writer, indent);
    }

    Writer_format(writer, ")");
}

static void SimpleTypeExpr_dump_internal(
    SimpleTypeExpr const* expr, Writer* writer, int indent
) {
    (void)indent; /* unused */
    switch (expr->kind) {
    #define X(name)                                   \
        case SimpleTypeKind_##name:                   \
            Writer_format(writer, "%sExpr()", #name); \
            break;
    SIMPLE_TYPE_LIST(X)
    #undef X
    }
}

static void FunctionTypeExpr_dump_internal(
    FunctionTypeExpr const* expr, Writer* writer, int indent
) {
    Writer_format(writer, "FunctionTypeExpr(\n");
    indent += 1;

    write_indent(writer, indent);
    Writer_format(writer, "return_type = ");
    Expr_dump_internal(expr->return_type, writer, indent);
    Writer_format(writer, ",\n");

    if (expr->base.type != NULL) {
        write_indent(writer, indent);
        Writer_format(writer, "type = ");
        Type_dump_internal(expr->base.type, writer, indent);
        Writer_format(writer, ",\n");
    }

    indent -= 1;
    write_indent(writer, indent);
    Writer_format(writer, ")");
}

/*
 * Types
 */

static void SimpleType_dump_internal(
    SimpleType const* type, Writer* writer, int indent
) {
    (void)indent; /* unused */
    switch (type->kind) {
    #define X(name)                               \
        case SimpleTypeKind_##name:               \
            Writer_format(writer, "%s()", #name); \
            break;
    SIMPLE_TYPE_LIST(X)
    #undef X
    }
}

static void FunctionType_dump_internal(
    FunctionType const* type, Writer* writer, int indent
) {
    Writer_format(writer, "FunctionType(\n");
    indent += 1;

    write_indent(writer, indent);
    Writer_format(writer, "return_type = ");
    Type_dump_internal(type->return_type, writer, indent);
    Writer_format(writer, ",\n");

    indent -= 1;
    write_indent(writer, indent);
    Writer_format(writer, ")");
}
