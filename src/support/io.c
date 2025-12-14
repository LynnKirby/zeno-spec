#include "src/support/io.h"
#include "src/support/malloc.h"

#include <string.h>

SystemIoError Writer_write_str(Writer* writer, StringRef string) {
    return writer->write(writer, string.data, string.size);
}

SystemIoError Writer_write_zstr(Writer* writer, char const* string) {
    return writer->write(writer, string, strlen(string));
}

SystemIoError Writer_write_bstr(Writer* writer, ByteStringRef string) {
    return writer->write(writer, string.data, string.size);
}

static SystemIoError FileWriter_write(
    Writer* writer, void const* data, size_t size
) {
    SystemFile system_file;
    system_file = ((FileWriter*)writer)->system_file;
    return SystemFile_write(system_file, data, size);
}

SystemIoError SystemFile_read_all(SystemFile file, void** data, size_t* size) {
    size_t capacity;
    *size = 0;

    capacity = 512;
    *data = xmalloc(capacity);

    for (;;) {
        SystemIoError res;
        size_t chunk_read;

        if (capacity - *size == 0) {
            /* FIXME: overflow */
            capacity += capacity / 2;
            *data = xrealloc(*data, capacity);
        }

        res = SystemFile_read(
            file, (char*)*data + *size, sizeof(capacity - *size), &chunk_read
        );

        if (res != SystemIoError_Success) {
            xfree(*data);
            return res;
        }

        *size += chunk_read;

        if (chunk_read == 0) {
            /* Nul-terminate */
            if (capacity - *size == 0) {
                /* FIXME: overflow */
                *data = xrealloc(*data, capacity + 1);
            }
            ((char*)*data)[*size] = 0;

            return SystemIoError_Success;
        }
    }
}

/*
 * Unix system IO.
 */

#ifdef __unix__

#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

SystemIoError SystemFile_read(
    SystemFile file,
    void* data,
    size_t size,
    size_t* out_size_read
) {
    *out_size_read = 0;

    for (;;) {
        ssize_t nread;

        nread = read(file, data, size);

        if (nread == 0) {
            return 0;
        }

        if (nread == -1) {
            if (errno == EINTR) continue;
            return errno;
        }

        data = (char*)data + nread;
        size -= nread;
        *out_size_read += nread;
    }
}

SystemIoError SystemFile_write(SystemFile file, void const* data, size_t size) {
    for (;;) {
        ssize_t written;

        if (size == 0) {
            return 0;
        }

        written = write(file, data, size);

        if (written == -1) {
            if (errno == EINTR) continue;
            return errno;
        }

        data = (char const*)data + written;
        size -= written;
    }
}

SystemIoError SystemFile_open_read(SystemFile* file, char const* path) {
    int fd;
    int flags = O_RDONLY | O_NOCTTY;
    struct stat statbuf;

    for (;;) {
        fd = open(path, flags);
        if (fd < 0) {
            if (errno == EINTR) continue;
            return errno;
        }
        break;
    }

    /* Check that we opened a file. */
    for (;;) {
        int res;
        res = fstat(fd, &statbuf);
        if (res != 0) {
            close(fd);
            return errno;
        }
        break;
    }
    if (S_ISDIR(statbuf.st_mode)) {
        close(fd);
        return EISDIR;
    }

    *file = fd;
    return 0;
}

int SystemFile_isatty(SystemFile file) {
    return isatty(file);
}

FileWriter FileWriter_stdout = {
    {FileWriter_write},
    SystemFile_stdout
};

FileWriter FileWriter_stderr = {
    {FileWriter_write},
    SystemFile_stderr
};

Writer* const Writer_stdout = &FileWriter_stdout.base;
Writer* const Writer_stderr = &FileWriter_stderr.base;

#endif /* __unix__ */

