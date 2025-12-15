#ifndef _ZENO_SPEC_SRC_AST_STRING_H
#define _ZENO_SPEC_SRC_AST_STRING_H

#include "src/support/stdint.h"
#include "src/support/string_ref.h"

/** String owned by an AstContext. */
typedef struct AstString {
    StringRef value;
    uint32_t hash;
} AstString;

void AstString_init(AstString* string, StringRef value);

int AstString_equal(AstString const* left, AstString const* right);
uint32_t AstString_hash(AstString const* item);

int AstString_equal_generic(void const* left, void const* right);
uint32_t AstString_hash_generic(void const* item);

#endif
