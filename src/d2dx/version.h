/*
	This file is part of D2DX.

	Copyright (C) 2026  Jack Rayner

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
