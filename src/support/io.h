#ifndef _ZENO_SPEC_SRC_SUPPORT_IO_H
#define _ZENO_SPEC_SRC_SUPPORT_IO_H

#include "src/support/base.h"

#include <stdarg.h>

/*
 * System IO.
 */

#if defined(__unix__)
    typedef int SystemFile;
    typedef int SystemIoError;
    #define SystemIoError_Success 0
    #define SystemFile_stdin 0
    #define SystemFile_stdout 1
    #define SystemFile_stderr 2
#else
    #error "implement me"
#endif

SystemIoError SystemFile_read(
    SystemFile file,
    void* data,
    size_t size,
    size_t* out_size_read
);

SystemIoError SystemFile_write(
    SystemFile file,
    void const* data,
    size_t size
);

SystemIoError SystemFile_open_read(SystemFile* file, char const* path);

int SystemFile_isatty(SystemFile file);

/*
 * Writer interface.
 */

typedef struct Writer {
    SystemIoError (*write)(
        struct Writer* writer,
        void const* data,
        size_t size
    );
} Writer;

typedef struct FileWriter {
    Writer base;
    SystemFile system_file;
    /* TODO: buffering */
} FileWriter;

extern Writer* const Writer_stdout;
extern Writer* const Writer_stderr;

/** Write byte buffer. */
static inline SystemIoError Writer_write(
    Writer* writer, void const* data, size_t size
) {
    return writer->write(writer, data, size);
}

/** Write formatted string. */
SystemIoError Writer_print(Writer* writer, char const* format, ...);

/** Write formatted string with va_list. */
SystemIoError Writer_vprint(Writer* writer, char const* format, va_list args);

/** Write string. */
SystemIoError Writer_write_str(Writer* writer, StringRef string);

/** Write nul-terminated byte string. */
SystemIoError Writer_write_zstr(Writer* writer, char const* string);

/** Write byte string. */
SystemIoError Writer_write_bstr(Writer* writer, ByteStringRef string);

/** Write signed integer. */
SystemIoError Writer_write_int(Writer* writer, intmax_t value, int base);

/** Write unsigned integer. */
SystemIoError Writer_write_uint(Writer* writer, intmax_t value, int base);

#endif
