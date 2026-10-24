// Minimal stand-in for Microsoft's CppUnitTest.h: just the subset used by src/d2dxtests.
#pragma once

#include <exception>
#include <string>
#include <type_traits>
#include <vector>

namespace Microsoft { namespace VisualStudio { namespace CppUnitTestFramework {

// Primary template; tests specialize it for their own types (used only for messages).
// static: the tests specialize it as `template<> static ...` (an MSVC-ism).
template<typename T>
static std::wstring ToString(const T& t)
{
	if constexpr (std::is_arithmetic_v<T>)
		return std::to_wstring(t);
	else if constexpr (std::is_enum_v<T>)
		return std::to_wstring((long long)t);
	else
		return L"<value>";
}

struct AssertFailure : std::exception
{
	std::string msg;
	explicit AssertFailure(std::string m) : msg(std::move(m)) {}
	const char* what() const noexcept override { return msg.c_str(); }
};

namespace detail {
inline std::string Narrow(const std::wstring& w)
{
	std::string s;
	for (wchar_t c : w) s.push_back(c < 128 ? (char)c : '?');
	return s;
}
}

struct Assert
{
	template<typename E, typename A, typename... Rest>
	static void AreEqual(const E& expected, const A& actual, Rest...)
	{
		if (!(expected == actual))
			throw AssertFailure("AreEqual failed. Expected:<" + detail::Narrow(ToString(expected)) +
				"> Actual:<" + detail::Narrow(ToString(actual)) + ">");
	}
	template<typename... Rest>
	static void IsTrue(bool v, Rest...) { if (!v) throw AssertFailure("IsTrue failed."); }
	template<typename... Rest>
	static void IsFalse(bool v, Rest...) { if (v) throw AssertFailure("IsFalse failed."); }
};

}}}

// Test registry used by main.cpp.
namespace cut {
struct Case { const char* name; void (*fn)(); };
inline std::vector<Case>& Registry() { static std::vector<Case> r; return r; }
inline bool Add(const char* name, void (*fn)()) { Registry().push_back({ name, fn }); return true; }
template<typename T> struct Base { using Self = T; };
}

#define TEST_CLASS(name) struct name : ::cut::Base<name>
#define TEST_METHOD(m) \
	static void _cut_run_##m() { Self t; t.m(); } \
	static inline const bool _cut_reg_##m = ::cut::Add(#m, &_cut_run_##m); \
	void m()
