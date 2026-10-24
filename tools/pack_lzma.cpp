/*
	This file is part of D2DX.

	Copyright (C) 2026  The D2DX contributors

	D2DX is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	D2DX is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with D2DX.  If not, see <https://www.gnu.org/licenses/>.
*/

// Packs and unpacks the .lzma files that d2dx.rc embeds (the SGD2FreeRes DLL
// and MPQ), using the same PocketLzma library that d2dx uses to unpack them.
//
//   pack_lzma <in> <out>       compress <in> into <out>
//   pack_lzma -d <in> <out>    decompress <in> into <out>
//
// Uses the BestCompression preset, which reproduces the original packing of
// the existing .lzma files byte for byte.
//
// Build as 32-bit and unoptimised (see AGENTS.md for the container commands):
//   clang++ -m32 -O0 -std=c++17 tools/pack_lzma.cpp -o build/pack_lzma
// PocketLzma shifts a 32-bit value by up to 56 bits when it reads and writes
// the size header, which is undefined behaviour. The existing files were packed
// by a 32-bit build, which writes the size twice; a 64-bit or optimised build
// reads that header wrongly and crashes when unpacking them.

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <iterator>
#include <vector>

#define POCKETLZMA_LZMA_C_DEFINE
#include "../thirdparty/pocketlzma/pocketlzma.hpp"

static bool ReadWholeFile(const char* path, std::vector<uint8_t>& data)
{
	std::ifstream f(path, std::ios::binary);
	if (!f)
	{
		return false;
	}
	data.assign(std::istreambuf_iterator<char>(f), std::istreambuf_iterator<char>());
	return !f.bad();
}

static bool WriteWholeFile(const char* path, const std::vector<uint8_t>& data)
{
	std::ofstream f(path, std::ios::binary | std::ios::trunc);
	if (!f)
	{
		return false;
	}
	f.write(reinterpret_cast<const char*>(data.data()), static_cast<std::streamsize>(data.size()));
	f.flush();
	return static_cast<bool>(f);
}

int main(int argc, char** argv)
{
	bool decompress = argc == 4 && std::strcmp(argv[1], "-d") == 0;
	if (argc != 3 && !decompress)
	{
		std::fprintf(stderr, "usage: pack_lzma <in> <out>\n       pack_lzma -d <in> <out>\n");
		return 2;
	}

	const char* inPath = argv[decompress ? 2 : 1];
	const char* outPath = argv[decompress ? 3 : 2];

	std::vector<uint8_t> input;
	if (!ReadWholeFile(inPath, input) || input.empty())
	{
		std::fprintf(stderr, "error: cannot read (or empty) input file: %s\n", inPath);
		return 1;
	}

	std::vector<uint8_t> output;
	plz::PocketLzma p(plz::Preset::BestCompression);
	plz::StatusCode status = decompress ? p.decompress(input, output) : p.compress(input, output);
	if (status != plz::StatusCode::Ok)
	{
		std::fprintf(stderr, "error: %s failed with status %d\n", decompress ? "decompress" : "compress", static_cast<int>(status));
		return 1;
	}

	if (!WriteWholeFile(outPath, output))
	{
		std::fprintf(stderr, "error: cannot write output file: %s\n", outPath);
		return 1;
	}

	return 0;
}
