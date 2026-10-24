#!/usr/bin/env python3
# This file is part of D2DX.
#
# Copyright (C) 2026  The D2DX contributors
#
# D2DX is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# D2DX is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with D2DX.  If not, see <https://www.gnu.org/licenses/>.

"""Make the official SGD2FreeRes 3.0.3.1 DLL read D2DX's config file.

D2DX writes its own SGD2FreeRes files (d2dx_sgd2freeres.json/.dll/.mpq) so it never
overwrites an SGD2FreeRes installation the player already has. The official DLL hard-codes
the config file name "SGD2FreeResolution.json", so it would ignore D2DX's config. The one
change made here replaces that file name, in place, with "d2dx_sgd2freeres.json" (shorter,
padded with NULs, so no offsets move). This is the same change as in the 3.0.2.0 build D2DX
shipped before: that DLL equals the official 3.0.2.0 DLL except for these 23 bytes.

Usage:
    patch_sgd2freeres.py <official SGD2FreeRes.dll> <patched output.dll>
    patch_sgd2freeres.py --verify <official SGD2FreeRes.dll> <patched.dll>

The input must be the DLL from the official 3.0.3.1 release; its SHA-256 is checked.
See thirdparty/sgd2freeres/MODIFICATIONS.md.
"""

import hashlib
import sys

OFFICIAL_SHA256 = "ad59e227dfe1a4a1d26ce5f6b717a0ae27858656d14bf31ca8efb63becc50d39"
OLD_NAME = b"SGD2FreeResolution.json"
NEW_NAME = b"d2dx_sgd2freeres.json"


def patch(data):
    if hashlib.sha256(data).hexdigest() != OFFICIAL_SHA256:
        raise SystemExit("input is not the official SGD2FreeRes 3.0.3.1 DLL (SHA-256 mismatch)")
    if data.count(OLD_NAME) != 1:
        raise SystemExit("expected exactly one occurrence of %r, found %d" % (OLD_NAME, data.count(OLD_NAME)))
    assert len(NEW_NAME) <= len(OLD_NAME)
    offset = data.index(OLD_NAME)
    replacement = NEW_NAME + b"\x00" * (len(OLD_NAME) - len(NEW_NAME))
    out = data[:offset] + replacement + data[offset + len(OLD_NAME):]
    assert len(out) == len(data)
    return out, offset


def main(argv):
    verify = len(argv) == 4 and argv[1] == "--verify"
    if not verify and len(argv) != 3:
        sys.stderr.write(__doc__)
        return 2
    official = open(argv[-2], "rb").read()
    expected, offset = patch(official)
    if verify:
        actual = open(argv[-1], "rb").read()
        if actual != expected:
            sys.stderr.write("MISMATCH: %s is not the official DLL plus the one-string patch\n" % argv[-1])
            return 1
        print("OK: %s is the official 3.0.3.1 DLL with only the file name at 0x%x changed" % (argv[-1], offset))
    else:
        open(argv[-1], "wb").write(expected)
        print("patched %r -> %r at 0x%x; output SHA-256 %s" % (
            OLD_NAME.decode(), NEW_NAME.decode(), offset, hashlib.sha256(expected).hexdigest()))
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))
