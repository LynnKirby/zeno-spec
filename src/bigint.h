#ifndef _ZENO_SPEC_SRC_BIGINT_H
#define _ZENO_SPEC_SRC_BIGINT_H

#include "src/base.h"
#include "src/io.h"

typedef struct BigInt {
#if UINTPTR_MAX < UINT64_MAX
    int64_t opaque;
#else
    intptr_t opaque;
#endif
} BigInt;

BigInt BigInt_from_int(intmax_t value);
BigInt BigInt_from_uint(uintmax_t value);

/** Loosely parse sequence of `base` digits. Ignores non-base characters. */
BigInt BigInt_parse(ByteStringRef string, int base);

void BigInt_destroy(BigInt* bigint);

/** Write big integer. */
SystemIoError BigInt_write(Writer* writer, BigInt bigint, int base);

#endif
