#ifndef _ZENO_SPEC_SRC_AST_DUMP_H
#define _ZENO_SPEC_SRC_AST_DUMP_H

#include "src/ast/nodes.h"

void Item_dump(Item const* expr, Writer* writer);
void Expr_dump(Expr const* expr, Writer* writer);
void Type_dump(Type const* type, Writer* writer);

#define X(name) void name##Item_dump(name##Item const* expr, Writer* writer);
ITEM_KIND_LIST(X)
#undef X

#define X(name) void name##Expr_dump(name##Expr const* expr, Writer* writer);
EXPR_KIND_LIST(X)
#undef X

#define X(name) void name##Type_dump(name##Type const* type, Writer* writer);
TYPE_KIND_LIST(X)
#undef X

#endif
