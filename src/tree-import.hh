#pragma once

#pragma GCC diagnostic push
#ifdef __clang__
#pragma GCC diagnostic ignored "-Wdocumentation"
#pragma GCC diagnostic ignored "-Wdocumentation-unknown-command"
#endif
#define lzma_nothrow
#include <lzma.h>
#pragma GCC diagnostic pop

#include "read-file.hh"
#include "newick.hh"

// ----------------------------------------------------------------------

const unsigned char sXzSig[] = { 0xFD, '7', 'z', 'X', 'Z', 0x00 };
constexpr ssize_t sXzBufSize = 409600;

inline std::string decompress_xz(std::string buffer)
{
    lzma_stream strm = LZMA_STREAM_INIT; /* alloc and init lzma_stream struct */
    if (lzma_stream_decoder(&strm, UINT64_MAX, LZMA_TELL_UNSUPPORTED_CHECK | LZMA_CONCATENATED) != LZMA_OK) {
        throw std::runtime_error("lzma decompression failed 1");
    }

    strm.next_in = reinterpret_cast<const uint8_t *>(buffer.c_str());
    strm.avail_in = buffer.size();
    std::string output(sXzBufSize, ' ');
    ssize_t offset = 0;
    for (;;) {
        strm.next_out = reinterpret_cast<uint8_t *>(&*(output.begin() + offset));
        strm.avail_out = sXzBufSize;
        auto const r = lzma_code(&strm, LZMA_FINISH);
        if (r == LZMA_STREAM_END) {
            output.resize(static_cast<size_t>(offset + sXzBufSize) - strm.avail_out);
            break;
        }
        else if (r == LZMA_OK) {
            offset += sXzBufSize;
            output.resize(static_cast<size_t>(offset + sXzBufSize));
        }
        else {
            throw std::runtime_error("lzma decompression failed 2");
        }
    }
    lzma_end(&strm);
    return output;
}

// ----------------------------------------------------------------------

class TreeImage;

inline void import_tree(Tree& tree, std::string buffer, TreeImage& aTreeImage)
{
    if (buffer == "-")
        buffer = read_stdin();
    else if (file_exists(buffer))
        buffer = read_file(buffer);
    if (std::memcmp(buffer.c_str(), sXzSig, sizeof(sXzSig)) == 0) {
        buffer = decompress_xz(buffer);
    }
    if (buffer[0] == '(')
        parse_newick(tree, std::begin(buffer), std::end(buffer));
    else if (buffer[0] == '{')
        tree_from_json(tree, buffer, aTreeImage);
    else
        throw std::runtime_error("cannot import tree: unrecognized source format");
}

// ----------------------------------------------------------------------
