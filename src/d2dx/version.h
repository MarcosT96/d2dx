// Single source of truth for the D2DX version. Used by d2dx.rc (VERSIONINFO).
//
// The three numbers are rewritten by release-please when a release PR is created
// (the markers at the end of each line are what it looks for), so don't edit them by hand
// and keep each #define on one line. This file is included from the resource compiler,
// so keep it to preprocessor directives only.
#pragma once

#define D2DX_VERSION_MAJOR 0 // x-release-please-major
#define D2DX_VERSION_MINOR 0 // x-release-please-minor
#define D2DX_VERSION_PATCH 1 // x-release-please-patch

#define D2DX_STRINGIZE_(x) #x
#define D2DX_STRINGIZE(x) D2DX_STRINGIZE_(x)

// "major.minor.patch.0", e.g. "0.0.1.0". No whitespace between the tokens, so the
// stringized result has none either.
#define D2DX_VERSION_STR D2DX_STRINGIZE(D2DX_VERSION_MAJOR.D2DX_VERSION_MINOR.D2DX_VERSION_PATCH.0)
