// Linux stand-in for src/d2dx/pch.h. Provides just enough of the Windows/D3D/MSVC
// surface for the logic sources covered by tests/linux to compile.
#pragma once

#include <stdint.h>

// hash_32a.c is C and includes pch.h too; nothing below applies to it.
#ifdef __cplusplus

#include <array>
#include <cassert>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <cstring>
#include <algorithm>
#include <chrono>
#include <exception>
#include <type_traits>
#include <vector>
#include <filesystem>
#include <memory>
#include <stdexcept>
#include <string>
#include <emmintrin.h>

// MSVC-isms.
#define __declspec(x)
#define abstract
#define __FUNCTION__ __func__
#define _Use_decl_annotations_
#define _aligned_free free
inline void* _aligned_malloc(size_t size, size_t alignment)
{
	void* p = nullptr;
	return posix_memalign(&p, alignment, size) == 0 ? p : nullptr;
}
inline char* _strdup(const char* s) { return strdup(s); }

// SAL annotations.
#define _In_
#define _Out_
#define _Inout_
#define _In_z_
#define _In_opt_
#define _Out_opt_
#define _Inout_updates_(n)
#define _In_reads_(n)
#define _In_reads_bytes_(n)
#define _Out_writes_(n)
#define _Out_writes_bytes_(n)
#define _Out_writes_all_(n)
#define _Success_(x)
#define _Ret_z_
#define _Ret_notnull_
#define _Must_inspect_result_

template<size_t N, typename... A>
int sprintf_s(char (&buf)[N], const char* fmt, A... args)
{
	return snprintf(buf, N, fmt, args...);
}

// Windows types used in the headers under test.
typedef long HRESULT;
#define FAILED(hr) ((hr) < 0)
#define SUCCEEDED(hr) ((hr) >= 0)
typedef unsigned long DWORD; // 32-bit under -m32, as on Win32
template<typename T>
inline unsigned char BitScanReverse(T* index, uint32_t mask)
{
	if (!mask) return 0;
	*index = 31 - __builtin_clz(mask);
	return 1;
}
template<typename T>
inline unsigned char BitScanReverse64(T* index, uint64_t mask)
{
	if (!mask) return 0;
	*index = 63 - __builtin_clzll(mask);
	return 1;
}
template<typename T>
inline unsigned char BitScanForward(T* index, uint32_t mask)
{
	if (!mask) return 0;
	*index = __builtin_ctz(mask);
	return 1;
}
template<typename T>
inline unsigned char BitScanForward64(T* index, uint64_t mask)
{
	if (!mask) return 0;
	*index = __builtin_ctzll(mask);
	return 1;
}
#define ARRAYSIZE(a) (sizeof(a) / sizeof((a)[0]))
typedef uint16_t WORD;
typedef uint8_t BYTE;
typedef void* HWND;
typedef void* HANDLE;

// D3D11 types are only ever passed as null pointers by the tests.
struct ID3D11Device;
struct ID3D11DeviceContext;
struct ID3D11Texture2D;
struct ID3D11ShaderResourceView;

namespace Microsoft { namespace WRL {
template<typename T>
class ComPtr
{
public:
	T* Get() const { return _p; }
	T** GetAddressOf() { return &_p; }
	T* operator->() const { return _p; }
	void Reset() { _p = nullptr; }
private:
	T* _p = nullptr;
};
}}

template<typename T>
using ComPtr = Microsoft::WRL::ComPtr<T>;

// Metrics.cpp does `using namespace DirectX;`.
namespace DirectX {}

// windows.h defines these as macros (the sources rely on mixed-type min/max). Keep
// last so they don't break std headers above.
#define min(a, b) (((a) < (b)) ? (a) : (b))
#define max(a, b) (((a) > (b)) ? (a) : (b))

#endif // __cplusplus
