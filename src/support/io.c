#include "src/support/io.h"

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

