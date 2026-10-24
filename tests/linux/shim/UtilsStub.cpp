// Linux stand-in for src/d2dx/Utils.cpp (which is all Win32). Implements only what
// the sources under test link against.
#include "pch.h"
#include "../../../src/d2dx/Utils.h" // explicit: shim/Utils.h would shadow it here

#include <chrono>

void d2dx::detail::Log(const char*) {}

int64_t d2dx::TimeStart()
{
	return std::chrono::steady_clock::now().time_since_epoch().count();
}

float d2dx::TimeEndMs(int64_t start)
{
	return (float)((TimeStart() - start) / 1e6);
}

char* d2dx::detail::GetMessageForHRESULT(HRESULT, const char*, int32_t) noexcept
{
	return (char*)"HRESULT failure";
}

void d2dx::detail::ThrowFromHRESULT(HRESULT hr, const char* func, int32_t line)
{
	throw d2dx::ComException(hr, func, line);
}

void d2dx::detail::FatalException() noexcept { std::abort(); }
void d2dx::detail::FatalError(const char*) noexcept { std::abort(); }
