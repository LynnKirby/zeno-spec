#ifndef _ZENO_SPEC_SRC_SUPPORT_ARRAY_WRITER_H
#define _ZENO_SPEC_SRC_SUPPORT_ARRAY_WRITER_H

#include "src/support/io.h"

typedef struct ArrayWriter {
    Writer base;
    uint8_t* data;
    size_t size;
    size_t capacity;
} ArrayWriter;

void ArrayWriter_init(ArrayWriter* writer);
void ArrayWriter_destroy(ArrayWriter* writer);
void ArrayWriter_reset(ArrayWriter* writer);

#endif
