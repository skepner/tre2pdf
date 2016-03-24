#pragma once

#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <cerrno>
#include <cstring>
#include <stdexcept>

// ----------------------------------------------------------------------

inline std::string read_file(std::string aFilename)
{
    std::string buffer;
    int f = open(aFilename.c_str(), O_RDONLY);
    if (f >= 0) {
        struct stat st;
        fstat(f, &st);
        buffer.resize(static_cast<std::string::size_type>(st.st_size), ' '); // reserve space
        read(f, &*buffer.begin(), static_cast<size_t>(st.st_size));
        close(f);
    }
    else {
        throw std::runtime_error(std::string("Cannot open ") + aFilename + ": " + strerror(errno));
    }
    return buffer;
}

// ----------------------------------------------------------------------

inline bool file_exists(std::string aFilename)
{
    struct stat buffer;
    return stat(aFilename.c_str(), &buffer) == 0;
}

// ----------------------------------------------------------------------

inline std::string read_from_file_descriptor(int fd, size_t chunk_size = 1024)
{
    std::string buffer;
    std::string::size_type offset = 0;
    for (;;) {
        buffer.resize(buffer.size() + chunk_size, ' ');
        const auto bytes_read = read(fd, (&*buffer.begin()) + offset, chunk_size);
        if (bytes_read < 0)
            throw std::runtime_error(std::string("Cannot read from file descriptor: ") + strerror(errno));
        if (static_cast<size_t>(bytes_read) < chunk_size) {
            buffer.resize(buffer.size() - chunk_size + static_cast<size_t>(bytes_read));
            break;
        }
        offset += static_cast<size_t>(bytes_read);
    }
    return buffer;
}

// ----------------------------------------------------------------------

inline std::string read_stdin()
{
    return read_from_file_descriptor(0);
}

// ----------------------------------------------------------------------
