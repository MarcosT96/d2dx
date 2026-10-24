cmake_minimum_required(VERSION 3.16)
project(d2dx_tests_linux CXX C)

# Portable subset of src/d2dxtests: compiles the pure-logic sources against small
# shims for the Windows headers and MS CppUnitTest. The Windows build (MSVC) stays
# authoritative; this is a fast local check only.

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

get_filename_component(ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../.." ABSOLUTE)
set(D2DX "${ROOT}/src/d2dx")
set(TESTS "${ROOT}/src/d2dxtests")

# A quoted `#include "pch.h"` is resolved relative to the including file first, so
# sources living in src/d2dx would pick up the real Windows pch.h. Compile copies from
# the build dir instead, so the shim directory (-I) wins.
set(COPIED_SOURCES)
foreach(f SimdSse2 Metrics TextureCache TextureCachePolicyBitPmru)
	configure_file(${D2DX}/${f}.cpp ${CMAKE_BINARY_DIR}/copied/${f}.cpp COPYONLY)
	list(APPEND COPIED_SOURCES ${CMAKE_BINARY_DIR}/copied/${f}.cpp)
endforeach()

add_executable(d2dx_tests
	${TESTS}/TestBatch.cpp
	${TESTS}/TestMetrics.cpp
	${TESTS}/TestSimd.cpp
	${TESTS}/TestTextureCache.cpp
	${COPIED_SOURCES}
	${CMAKE_CURRENT_SOURCE_DIR}/shim/UtilsStub.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/shim/main.cpp
	${ROOT}/thirdparty/fnv/hash_32a.c)

# shim must come first so its pch.h shadows src/d2dx/pch.h.
target_include_directories(d2dx_tests BEFORE PRIVATE
	${CMAKE_CURRENT_SOURCE_DIR}/shim)
target_include_directories(d2dx_tests PRIVATE
	${D2DX}
	${ROOT}/thirdparty/fnv
	${ROOT}/thirdparty/glide3)
target_compile_definitions(d2dx_tests PRIVATE D2DX_UNITTEST
	KNOWN_FAILURES_FILE="${CMAKE_CURRENT_SOURCE_DIR}/known-failures.txt")
target_compile_options(d2dx_tests PRIVATE -m32 -msse2 -mfpmath=sse -ffp-contract=off -fms-extensions -w)
target_link_options(d2dx_tests PRIVATE -m32)

enable_testing()
add_test(NAME d2dx_tests COMMAND d2dx_tests)
