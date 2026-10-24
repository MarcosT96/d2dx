// Wraps src/d2dx/Utils.h. D2DX_LOG(fmt) with no extra args expands to a trailing comma,
// which MSVC's preprocessor swallows but clang does not.
#pragma once
#include_next "Utils.h"

#undef D2DX_LOG
#define D2DX_LOG(fmt, ...) \
	{ \
		static char ssss[256]; \
		sprintf_s(ssss, fmt "\n", ##__VA_ARGS__); \
		d2dx::detail::Log(ssss); \
	}
