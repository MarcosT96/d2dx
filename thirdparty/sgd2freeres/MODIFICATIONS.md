# Modifications to SGD2FreeRes in D2DX

D2DX embeds **SlashGaming Diablo II Free Resolution (SGD2FreeRes) 3.0.3.1** by Mir Drualga
(licensed under the GNU Affero General Public License v3 or later; see `SGD2FreeRes - LICENSE.md`
and `SGD2FreeRes - COPYING`). The embedded DLL is **modified**: one file-name string is changed.
Its complete corresponding source is the official source at tag `3.0.3.1` of
<https://github.com/mir-diablo-ii-tools/SlashGaming-Diablo-II-Free-Resolution>, plus the change
described here. `SGD2FreeRes.mpq` is the official file, unmodified.

## The change (made 2026-10-24)

The official DLL reads its settings from a file named `SGD2FreeResolution.json` in the game folder.
D2DX writes its own copies of the SGD2FreeRes files (`d2dx_sgd2freeres.json`, `.dll`, `.mpq`) at
startup, so that it never overwrites an SGD2FreeRes installation a player already has. The one
change is in the DLL's read-only data: the string `SGD2FreeResolution.json` (23 bytes) becomes
`d2dx_sgd2freeres.json` (21 bytes) followed by two NUL bytes, at file offset `0x307cc`. Nothing else
differs (23 bytes in total), and no offsets move.

The 3.0.2.0 DLL that D2DX embedded before was the official 3.0.2.0 DLL with the same change (checked
byte for byte: the only difference was the same 23 bytes, at `0x2f964`).

## Reproducing it

1. Download the official release asset `SGD2FreeRes.3.0.3.1.-.Vanilla.zip` (SHA-256
   `4469a1f8d5299e9af935d088324504be5eed948f49c26aa568797a77d1c28463`) from the release page of the
   repository above, and extract `SGD2FreeRes.dll` (SHA-256
   `ad59e227dfe1a4a1d26ce5f6b717a0ae27858656d14bf31ca8efb63becc50d39`).
2. `python3 tools/patch_sgd2freeres.py SGD2FreeRes.dll patched.dll` (it checks that hash first and
   that the string occurs exactly once). The output has SHA-256
   `408b1b5e0fd53295273a0c3af1745f4cac2554d73cf2e44f57337527c1c8bfa7`, which is the
   `SGD2FreeRes.dll` in this folder.
3. `python3 tools/patch_sgd2freeres.py --verify SGD2FreeRes.dll <the DLL in this folder>` confirms it.
4. `SGD2FreeRes.dll.lzma` is that DLL compressed with `tools/pack_lzma.cpp` (see `AGENTS.md`).
