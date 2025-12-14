#include "src/support/encoding.h"

#define is_utf8_continuation(byte) (((byte) & 0xC0) == 0x80)

int32_t utf8_decode(uint8_t const** p_cursor, uint8_t const* limit) {
    static const uint8_t lengths[] = {
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 2, 2, 3, 3, 4, 0
    };
    uint8_t length;
    uint8_t const* cursor;
    int32_t res;

    cursor = *p_cursor;

    if (cursor == limit) {
        return U_EOF;
    }

    length = lengths[*cursor >> 3];

    /* Reject non-continuation bytes. */
    {
        uint8_t i;
        for (i = 1; i < length; i += 1) {
            if (!is_utf8_continuation(cursor[i])) {
                *p_cursor += i;
                return U_BAD;
            }
        }
    }

    *p_cursor += length;

    switch (length) {
    default:
        *p_cursor += 1;
        return U_BAD;

    case 1:
        return cursor[0];

    case 2:
        return ((cursor[0] & 0x1F) << 6)
            | (cursor[1] & 0x3F);

    case 3:
        res = ((cursor[0] & 0x0F) << 12)
            | ((cursor[1] & 0x3F) << 6)
            | (cursor[2] & 0x3F);
        if (res < 0x800) {
            /* Overlong encoding. */
            return U_BAD;
        }
        if (res >= 0xD800 && res <= 0xDFFF) {
            /* Surrogate. */
            return U_BAD;
        }
        return res;

    case 4:
        res = ((cursor[0] & 0x07) << 18)
            | ((cursor[1] & 0x3F) << 12)
            | ((cursor[2] & 0x3F) << 6)
            | (cursor[3] & 0x3F);
        if (res > 0x10FFFF) {
            /* Out of range. */
            return U_BAD;
        }
        return res;
    }
}
