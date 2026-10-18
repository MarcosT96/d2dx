/*
    pack_lzma - repack a file into the PocketLzma container D2DX embeds.

    D2DX stores thirdparty/sgd2freeres/sgd2freeres.dll.lzma (and .mpq.lzma) as
    PocketLzma streams and decompresses them at runtime with BuiltinResMod.cpp
    -> Utils.cpp DecompressLZMAToFile. Building the stream with the very same
    PocketLzma header used to decompress it removes any guesswork about the
    container layout (5 bytes of LZMA properties + 8 bytes of uncompressed size
    + the raw LZMA1 stream).

    Usage: pack_lzma <input> <output>
*/

#define POCKETLZMA_LZMA_C_DEFINE
#include "pocketlzma.hpp"

#include <cstdio>
#include <fstream>
#include <iterator>
#include <vector>

int main(int argc, char** argv)
{
    if (argc != 3)
    {
        std::fprintf(stderr, "usage: %s <input> <output>\n", argv[0]);
        return 2;
    }

    std::ifstream in(argv[1], std::ios::binary);
    if (!in)
    {
        std::fprintf(stderr, "error: cannot open %s\n", argv[1]);
        return 1;
    }

    std::vector<uint8_t> data(
        (std::istreambuf_iterator<char>(in)),
        std::istreambuf_iterator<char>());

    plz::PocketLzma lzma;
    std::vector<uint8_t> packed;

    if (lzma.compress(data, packed) != plz::StatusCode::Ok)
    {
        std::fprintf(stderr, "error: compression failed\n");
        return 1;
    }

    // Round-trip with the same decompressor D2DX uses.
    std::vector<uint8_t> verify;
    if (lzma.decompress(packed, verify) != plz::StatusCode::Ok || verify != data)
    {
        std::fprintf(stderr, "error: round-trip verification failed\n");
        return 1;
    }

    std::ofstream out(argv[2], std::ios::binary);
    if (!out)
    {
        std::fprintf(stderr, "error: cannot write %s\n", argv[2]);
        return 1;
    }

    out.write(reinterpret_cast<const char*>(packed.data()),
        static_cast<std::streamsize>(packed.size()));
    out.close();

    if (!out)
    {
        std::fprintf(stderr, "error: write failed\n");
        return 1;
    }

    std::printf("packed %zu -> %zu bytes (round-trip ok)\n", data.size(), packed.size());
    return 0;
}
