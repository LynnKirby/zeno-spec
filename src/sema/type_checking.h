#ifndef _ZENOC_SRC_SEMA_TYPE_CHECKING_H
#define _ZENOC_SRC_SEMA_TYPE_CHECKING_H

#include "src/ast/context.h"
#include "src/ast/nodes.h"
#include "src/support/source_pos.h"

typedef enum TypeCheckResultKind {
    TypeCheckResultKind_Success,
    TypeCheckResultKind_UndeclaredName,
    TypeCheckResultKind_ExpectedType
} TypeCheckResultKind;

typedef struct UndeclaredName {
    StringRef name;
    SourcePos pos;
} UndeclaredName;

typedef struct ExpectedType {
    Type* actual;
    Type* expected;
    SourcePos pos;
} ExpectedType;

typedef struct TypeCheckResult {
    TypeCheckResultKind kind;
    union {
        UndeclaredName undeclared_name;
        ExpectedType expected_type;
    } as;
} TypeCheckResult;

void type_check(TypeCheckResult* result, AstContext* ast, FunctionItem* item);

#endif
