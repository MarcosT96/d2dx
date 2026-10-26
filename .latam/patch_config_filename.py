#!/usr/bin/env python3
"""Point the official SGD2FreeRes DLL at D2DX's generated config file.

D2DX writes its own copies of the SGD2FreeRes files at startup
(`d2dx_sgd2freeres.json/.dll/.mpq`) so it never touches an SGD2FreeRes
installation the player already has, then loads `d2dx_sgd2freeres.dll`. The
official DLL hard-codes the name `SGD2FreeResolution.json`, so it would ignore
D2DX's config and crash in the menus. This is the same single read-only-string
change D2DX itself makes (23 bytes, `nv`, padded with NULs so no offset moves).

Unlike D2DX's copy of the tool, this works on a DLL we built ourselves, so it
recomputes the offset instead of checking a known hash.

  patch_config_filename.py <in.dll> <out.dll>
"""

import hashlib
import sys

OLD_NAME = b"SGD2FreeResolution.json"
NEW_NAME = b"d2dx_sgd2freeres.json"


def main(argv):
    if len(argv) != 3:
        sys.stderr.write(__doc__)
        return 2
    data = open(argv[1], "rb").read()

    if data.count(OLD_NAME) != 1:
        raise SystemExit("expected exactly one %r, found %d" % (OLD_NAME, data.count(OLD_NAME)))
    if len(NEW_NAME) > len(OLD_NAME):
        raise SystemExit("replacement is longer; offsets would move")

    offset = data.index(OLD_NAME)
    replacement = NEW_NAME + b"\x00" * (len(OLD_NAME) - len(NEW_NAME))
    out = data[:offset] + replacement + data[offset + len(OLD_NAME):]
    if len(out) != len(data):
        raise SystemExit("size changed, which would move offsets")

    open(argv[2], "wb").write(out)
    print("patched %r -> %r at 0x%x" % (OLD_NAME, NEW_NAME, offset))
    print("output size %d, sha256 %s" % (len(out), hashlib.sha256(out).hexdigest()))
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))
