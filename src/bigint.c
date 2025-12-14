#include "src/bigint.h"
#include "src/io.h"

#include <assert.h>
#include <stdlib.h>

#define BIGINT_INLINE_MAX ((INT64_C(1) << 62) - 1)
#define BIGINT_INLINE_MIN (-BIGINT_INLINE_MAX - 1)

BigInt BigInt_from_int(intmax_t value) {
    BigInt bigint;
    if (value <= BIGINT_INLINE_MAX && value >= BIGINT_INLINE_MIN) {
        bigint.opaque = (value << 1) | 1;
    } else {
        assert(0 && "implement me");
    }
    return bigint;
}

BigInt BigInt_from_uint(uintmax_t value) {
    BigInt bigint;
    if (value <= (intmax_t)BIGINT_INLINE_MAX) {
        bigint.opaque = (value << 1) | 1;
    } else {
        assert(0 && "implement me");
    }
    return bigint;
}

void BigInt_destroy(BigInt* bigint) {
    if (bigint->opaque & 1) {
        return;
    }
    free((void*)bigint->opaque);
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
        assert(uvalue <= BIGINT_INLINE_MAX);
        /* FIXME proper overflow checking */
    }

    return BigInt_from_uint(uvalue);
}

SystemIoError BigInt_write(Writer* writer, BigInt bigint, int base) {
    assert(bigint.opaque & 1);
    return Writer_write_int(writer, bigint.opaque >> 1, base);
}
