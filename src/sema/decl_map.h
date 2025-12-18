#ifndef _ZENO_SPEC_SRC_SEMA_DECL_MAP_H
#define _ZENO_SPEC_SRC_SEMA_DECL_MAP_H

#include "src/ast/nodes.h"
#include "src/support/hash_map.h"

typedef enum DeclKind {
    DeclKind_Const
} DeclKind;

typedef struct ConstDecl {
    Expr* value;
    Type* type;
} ConstDecl;

typedef struct Decl {
    DeclKind kind;
    union {
        ConstDecl const_decl;
    } as;
} Decl;

typedef struct DeclMapNode {
    struct DeclMapNode* next;
    HashMap map;
} DeclMapNode;

/** Stack of scopes mapping name to declaration. */
typedef struct DeclMap {
    DeclMapNode* active;
    DeclMapNode* cache;
} DeclMap;

void DeclMap_init(DeclMap* decls);
void DeclMap_destroy(DeclMap* decls);

/** Push name scope. */
void DeclMap_push_scope(DeclMap* decls);

/** Pop name scope. */
void DeclMap_pop_scope(DeclMap* decls);

/** Get declaration by name, or NULL if not present. */
Decl const* DeclMap_get(DeclMap* decls, AstString name);

/** Set declaration by name. */
void DeclMap_set(DeclMap* decls, AstString name, Decl const* decl);

#endif
