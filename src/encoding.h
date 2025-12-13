#ifndef _ZENO_SPEC_SRC_ENCODING_H
#define _ZENO_SPEC_SRC_ENCODING_H

#include "src/defs.h"

#define U_EOF -1
#define U_BAD -2

/** Decode one UTF-8 character. Updates `cursor` with new location. */
int32_t utf8_decode(uint8_t const** cursor, uint8_t const* limit);

#endif
