#!/usr/bin/env python3
"""Insert the D2Latam realm into SGD2FreeRes' per-IP resolution allowlist.

SGD2FreeRes decides the in-game resolution set online by looking the gateway up
in `Ipv4HashResolutionTable`, keyed by SHA-1 of the realm's IP. Our realm is not
in the official table, so online we get the base modes only; the JSON config is
ignored in that path (see SelectLocalOrOnlineResolutions in the same file).

The table must stay sorted lexicographically by key. Our key, DE946264..., sorts
after C311F388... and before E1625F18..., so the entry goes immediately before
the E1625F18 entry, found by its key literal rather than by a comment.

  apply_allowlist.py <game_resolution.cc> v1   # 1068x600 + 1344x700 only
  apply_allowlist.py <game_resolution.cc> v2   # same, plus 800x600 as fallback
"""

import hashlib
import sys

KEY = "DE946264EB3B0B2A34D04F6D15C8524CD68E2FA5"
AFTER = 'C311F388012034C4ACB91AC573965302CF5711E0'
BEFORE = 'E1625F180F649ED2E4C98B4210012B6C52D9361F'

ENTRY_V1 = '''\n      // D2Latam - widescreen only
      Ipv4HashResolutionTableEntry(
          "{key}",
          {{
              std::make_tuple(1068, 600),
              std::make_tuple(1344, 700)
          }}
      ),

'''.format(key=KEY)

ENTRY_V2 = '''\n      // D2Latam - widescreen, with the engine's 800x600 kept as a fallback
      Ipv4HashResolutionTableEntry(
          "{key}",
          {{
              kResolution800x600,
              std::make_tuple(1068, 600),
              std::make_tuple(1344, 700)
          }}
      ),

'''.format(key=KEY)


def main(argv):
    if len(argv) != 3 or argv[2] not in ("v1", "v2"):
        sys.stderr.write(__doc__)
        return 2
    path, variant = argv[1], argv[2]
    text = open(path, encoding="utf-8", newline="").read()

    if KEY in text:
        raise SystemExit("the D2Latam entry is already present")

    if text.count(BEFORE) != 1:
        raise SystemExit("expected exactly one %s entry, found %d" % (BEFORE, text.count(BEFORE)))

    at = text.index(BEFORE)
    entry_start = text.rindex("Ipv4HashResolutionTableEntry(", 0, at)
    line_start = text.rindex("\n", 0, entry_start) + 1

    # Back up over the comment block that documents the *neighbouring* entry, so
    # our entry does not land between a comment and the entry it describes.
    insert_at = line_start
    while True:
        prev_end = insert_at - 1
        if prev_end <= 0:
            break
        prev_start = text.rindex("\n", 0, prev_end) + 1
        line = text[prev_start:insert_at].strip()
        if line.startswith("//") or line == "":
            insert_at = prev_start
            continue
        break

    # Keep an ASCII-sorted table honest: the key we insert must land between the
    # neighbours we claim, otherwise the lookup silently misses.
    if not (AFTER < KEY < BEFORE):
        raise SystemExit("key order assumption is wrong")

    entry = ENTRY_V1 if variant == "v1" else ENTRY_V2
    out = text[:insert_at] + entry + text[insert_at:]

    # The table is a std::array whose size is written by hand, so the entry and
    # the count have to move together or the last entries are "too many
    # initializers" (C2078) and nothing compiles.
    import re
    pat = re.compile(r"(std::array<\s*Ipv4HashResolutionTableEntry,\s*)(\d+)(\s*>)")
    matches = pat.findall(out)
    if len(matches) != 1:
        raise SystemExit("expected exactly one std::array<Ipv4HashResolutionTableEntry, N>, found %d" % len(matches))
    declared = int(matches[0][1])
    entries = out.count("Ipv4HashResolutionTableEntry(")
    if declared != entries - 1:
        raise SystemExit("table declares %d but has %d entries; refusing to guess" % (declared, entries - 1))
    out = pat.sub(lambda m: m.group(1) + str(declared + 1) + m.group(3), out, count=1)

    open(path, "w", encoding="utf-8", newline="").write(out)

    print("inserted %s entry for %s at byte %d" % (variant, KEY, insert_at))
    print("array size %d -> %d" % (declared, declared + 1))
    print("file sha256 now %s" % hashlib.sha256(out.encode("utf-8")).hexdigest())
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))
