#include "src/support/bigint.h"
#include "src/support/io.h"
#include "src/support/malloc.h"

#include <assert.h>
#include <stdlib.h>

#define BIGINT_INLINE_MAX ((INT64_C(1) << 62) - 1)
#define BIGINT_INLINE_MIN (-BIGINT_INLINE_MAX - 1)

BigInt BigInt_from_int(intmax_t value) {
    BigInt bigint;
    if (value <= BIGINT_INLINE_MAX && value >= BIGINT_INLINE_MIN) {
        bigint.opaque = (value << 1) | 1;
    } else {
        bigint.opaque = 0;
    }
    return bigint;
}

BigInt BigInt_from_uint(uintmax_t value) {
    BigInt bigint;
    if (value <= (intmax_t)BIGINT_INLINE_MAX) {
        bigint.opaque = (value << 1) | 1;
    } else {
        bigint.opaque = 0;
    }
    return bigint;
}

void BigInt_destroy(BigInt* bigint) {
    if (bigint->opaque & 1) {
        return;
    }
    /* xfree((void*)bigint->opaque); */
}

uint32_t BigInt_as_uint32(BigInt bigint) {
    return (uint32_t)bigint.opaque >> 1;
}

BigInt BigInt_parse(ByteStringRef string, int base) {
    uintmax_t uvalue;
    char const* cursor;
    char const* limit;

    uvalue = 0;
    cursor = string.data;
    limit = cursor + string.size;

    for (;;) {
        char ch;
        int digit;

        if (cursor == limit) {
            break;
        }

        ch = *cursor;
        cursor += 1;

        if (ch >= '0' && ch <= '9') {
            digit = ch - '0';
        } else if (ch >= 'a' && ch <= 'z') {
            digit = ch - 'a' + 10;
        } else if (ch >= 'A' && ch <= 'Z') {
            digit = ch - 'A' + 10;
        } else {
            continue;
        }

        if (digit >= base) {
            continue;
        }

        uvalue *= base;
        uvalue += digit;
        /* assert(uvalue <= BIGINT_INLINE_MAX); */
        /* FIXME proper overflow checking */
    }

    return BigInt_from_uint(uvalue);
}

SystemIoError BigInt_write(Writer* writer, BigInt bigint, int base) {
    if (bigint.opaque & 1) {
        return Writer_write_int(writer, bigint.opaque >> 1, base);
    } else {
        return Writer_format(writer, "<bigint>");
    }
}
