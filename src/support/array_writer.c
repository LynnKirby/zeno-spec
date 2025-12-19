#include "src/support/array_writer.h"
#include "src/support/malloc.h"

#include <string.h>

static SystemIoError ArrayWriter_write(
    Writer* base_writer, void const* data, size_t size
) {
    ArrayWriter* writer;
    writer = (ArrayWriter*)base_writer;
    writer->data = ensure_array_capacity(
        1, writer->data, &writer->size, &writer->capacity, size
    );
    memcpy(writer->data + writer->size, data, size);
    writer->size += size;
    return SystemIoError_Success;
}

void ArrayWriter_init(ArrayWriter* writer) {
    writer->base.write = ArrayWriter_write;
    writer->data = NULL;
    writer->size = 0;
    writer->capacity = 0;
}

void ArrayWriter_destroy(ArrayWriter* writer) {
    xfree(writer->data);
}

void ArrayWriter_reset(ArrayWriter* writer) {
    writer->size = 0;
}
