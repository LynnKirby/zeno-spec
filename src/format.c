#include "src/io.h"

#include <string.h>

#define WRITE(data, size) writer->write(writer, (data), (size));

SystemIoError Writer_print(Writer* writer, char const* format, ...) {
    va_list args;
    SystemIoError res;
    va_start(args, format);
    res = Writer_vprint(writer, format, args);
    va_end(args);
    return res;
}

typedef enum FormatOption {
    FormatOption_LeadingZero = 1 << 0,
    FormatOption_Uppercase = 1 << 1,
    FormatOption_IsSigned = 1 << 2
} FormatOption;

static SystemIoError print_number(
    Writer* writer,
    uintmax_t uvalue,
    int has_minus,
    int base,
    int width,
    FormatOption options
) {
    uint8_t buf[64];
    size_t size = 0;
    uint8_t* cursor;
    int extra_zeroes;

#if UINTMAX_MAX > UINT64_MAX
    #error FIXME
#endif

    cursor = buf + sizeof(buf);
    extra_zeroes = width;

    for (;;) {
        uint8_t digit;
        digit = uvalue % base;
        uvalue /= base;
        if (digit < 10) {
            digit += '0';
        } else if (options & FormatOption_Uppercase) {
            digit += 'A' - 10;
        } else {
            digit += 'a' - 10;
        }
        cursor -= 1;
        size += 1;
        *cursor = digit;
        if (extra_zeroes > 0) {
            extra_zeroes -= 1;
        }
        if (uvalue == 0) break;
    }

    if (has_minus) {
        int res;
        if (extra_zeroes > 0) {
            extra_zeroes -= 1;
        }
        res = WRITE("-", 1);
        if (res != SystemIoError_Success) {
            return res;
        }
    }

    while (extra_zeroes > 0) {
        int res;
        extra_zeroes -= 1;
        res = WRITE("0", 1);
        if (res != SystemIoError_Success) {
            return res;
        }
    }

    return WRITE(cursor, size);
}

static SystemIoError print_number_from_args(
    Writer* writer, va_list args, int base, int width, FormatOption options
) {
    uintmax_t uvalue;
    int has_minus;

    if (options & FormatOption_IsSigned) {
        intmax_t ivalue;
        ivalue = va_arg(args, int);
        if (ivalue < 0) {
            uvalue = -(uintmax_t)ivalue;
            has_minus = true;
        } else {
            uvalue = ivalue;
            has_minus = false;
        }
    } else {
        uvalue = va_arg(args, unsigned);
        has_minus = false;
    }

    return print_number(writer, uvalue, has_minus, base, width, options);
}

static int try_parse_int(char const** p_format) {
    int res = 0;

    for (;;) {
        char ch;
        ch = **p_format;
        if (ch >= '0' && ch <= '9') {
            *p_format += 1;
            res *= 10;
            res += ch - '0';
        } else {
            break;
        }
    }

    return res;
}

SystemIoError Writer_vprint(Writer* writer, char const* format, va_list args) {
    SystemIoError res = SystemIoError_Success;
    char const* chunk_start;
    FormatOption options;
    int width;

    chunk_start = format;

loop:
    if (*format == 0) goto done;

    if (*format != '%') {
        format += 1;
        goto loop;
    }

    /* Flush chunk. */
    if (chunk_start < format) {
        res = WRITE(chunk_start, format - chunk_start);
    }

    /* Skip past % */
    format += 1;

    /* Escaped %% */
    if (*format == '%') {
        /* Start next chunk on the percent character. */
        chunk_start = format;
        format += 1;
        goto loop;
    }

    options = 0;

    switch (*format) {
    case '0':
        options |= FormatOption_LeadingZero;
        format += 1;
        break;
    }

    width = try_parse_int(&format);

    switch (*format) {
    case 's': {
        char const* s;
        size_t len;
        s = va_arg(args, char const*);
        len = strlen(s);
        WRITE(s, len);
        format += 1;
        break;
    }

    case 'i': case 'd':
        options |= FormatOption_IsSigned;
        res = print_number_from_args(writer, args, 10, width, options);
        format += 1;
        break;

    case 'u':
        res = print_number_from_args(writer, args, 10, width, options);
        format += 1;
        break;

    case 'X':
        options |= FormatOption_Uppercase;
        /* fallthrough */
    case 'x':
        res = print_number_from_args(writer, args, 16, width, options);
        format += 1;
        break;

    default:
        /* TODO: assert bad format string */
        break;
    }

    chunk_start = format;
    goto loop;

done:
    /* Flush chunk at end. */
    if (chunk_start < format && res == SystemIoError_Success) {
        res = WRITE(chunk_start, format - chunk_start);
    }

    return res;
}

SystemIoError Writer_write_int(Writer* writer, intmax_t value, int base) {
    uintmax_t uvalue;

    int has_minus;

    if (value < 0) {
        uvalue = -(uintmax_t)value;
        has_minus = true;
    } else {
        uvalue = value;
        has_minus = false;
    }

    return print_number(
        writer, uvalue, has_minus, base, 0, FormatOption_Uppercase
    );
}

SystemIoError Writer_write_uint(Writer* writer, intmax_t value, int base) {
    return print_number(writer, value, false, base, 0, FormatOption_Uppercase);
}
