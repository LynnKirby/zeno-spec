#ifndef _ZENO_SPEC_SRC_SUPPORT_SOURCE_POS_H
#define _ZENO_SPEC_SRC_SUPPORT_SOURCE_POS_H

#include "src/support/stdint.h"

typedef struct SourcePos {
    uint32_t line;
    uint32_t column;
} SourcePos;

#endif
