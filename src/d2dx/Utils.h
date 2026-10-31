/*
	This file is part of D2DX.

	Copyright (C) 2021  Bolrog

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
#pragma once

#include "ErrorHandling.h"
#include "Buffer.h"

#define D2DX_LOG(fmt, ...) \
	{ \
		static char ssss[1024]; \
		sprintf_s(ssss, fmt "\n", __VA_ARGS__); \
		d2dx::detail::Log(ssss); \
	}

#ifdef NDEBUG
#define D2DX_DEBUG_LOG(fmt, ...)
#else
#define D2DX_DEBUG_LOG(fmt, ...) D2DX_LOG(fmt, __VA_ARGS__)
#endif

#ifdef D2DX_PROFILE
#define D2DX_LOG_PROFILE(fmt, ...) D2DX_LOG(fmt, __VA_ARGS__)
#else
#define D2DX_LOG_PROFILE(fmt, ...)
#endif

namespace d2dx
{
	namespace detail
	{
		__declspec(noinline) void Log(_In_z_ const char* s);
	}

	int64_t TimeStamp() noexcept;
	double TimeToMs(int64_t time) noexcept;

	struct WindowsVersion 
	{
		uint32_t major;
		uint32_t minor;
		uint32_t build;
	};

	WindowsVersion GetWindowsVersion();

	WindowsVersion GetActualWindowsVersion();

	Buffer<char> ReadTextFile(
		_In_z_ const char* filename);

	void DumpTexture(
		_In_ uint64_t hash,
		_In_ int32_t w,
		_In_ int32_t h,
		_In_reads_(pixelsSize) const uint8_t* pixels,
		_In_ uint32_t pixelsSize,
		_In_reads_(256) const uint32_t* palette);

	bool DecompressLZMAToFile(
		_In_reads_(dataSize) const uint8_t* data,
		_In_ uint32_t dataSize,
		_In_z_ const char* filename);

	/**
		Writes the file only when its content differs from `data`.

		Every running client shares the game directory, so the naive
		CREATE_ALWAYS + write is a data race with the other instances:
		WriteConfig opens d2dx_sgd2freeres.json with a share mode of 0, and a
		second client whose SGD2FreeRes reads the file inside that window gets a
		sharing violation - which its static config reader latches forever,
		leaving that client on the default 640x480/800x600 list.

		Writing through a temporary file and renaming it makes the replacement
		atomic (readers see the old or the new content, never a truncated file),
		and an up to date file is not opened for writing at all.
	*/
	bool WriteFileIfDifferent(
		_In_reads_(dataSize) const uint8_t* data,
		_In_ uint32_t dataSize,
		_In_z_ const char* filename);

	int32_t IndexOfUInt64(
		_In_reads_(itemsCount) const uint64_t* __restrict items,
		_In_ uint32_t itemsCount,
		_In_ uint64_t item);
}
