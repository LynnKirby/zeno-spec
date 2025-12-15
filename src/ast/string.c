#include "src/ast/string.h"

void AstString_init(AstString* string, StringRef value) {
    string->value = value;
    string->hash = StringRef_hash(value);
}

int AstString_equal(AstString const* left, AstString const* right) {
    return StringRef_equal(left->value, right->value);
}

uint32_t AstString_hash(AstString const* item) {
    return item->hash;
}

int AstString_equal_generic(void const* left, void const* right) {
    return AstString_equal(left, right);
}

uint32_t AstString_hash_generic(void const* item) {
    return AstString_hash(item);
}
