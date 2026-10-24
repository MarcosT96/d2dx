# Development tracker

What this fork is working on, what is next, and what we have decided not to do (with reasons, so it isn't re-litigated).
Update this file in the same PR that changes an item's status. Upstream = `bolrog/d2dx`; issue/PR numbers below are upstream's unless prefixed with `fork#`.

Research basis: a survey of upstream's forks, open PRs and issues on 2026-10-24. Mappings from fork changes to issues are **inferred from commit messages and PR descriptions, not tested**. "Fixes #N" claims are the original PR authors' own. Verify a fix in the game before closing anything.

## Done (this fork)
| Item | Where | Merged |
|---|---|---|
| Windows CI + containerised Linux/macOS unit-test runner | fork#2 | 2026-10-24 |
| `AGENTS.md` / `CLAUDE.md` agent guidance | fork#1 | 2026-10-24 |
| Fix `GetRenderRect` out-of-bounds rects (both `TestMetrics` known failures now pass) | fork#4 | 2026-10-24 |
| Run CodeQL on Windows with a manual MSBuild step | fork#7 | 2026-10-24 |
| Skip the Windows build/tests for docs-only changes; `Tests` workflow only on pushes to `main` | fork#9 | 2026-10-24 |

## In progress (this fork)
| Item | Where | Status |
|---|---|---|
| Dependabot for GitHub Actions | fork#3 | open |
| This tracker | fork#8 | open |

## Planned (priority order)
Source is mostly Jarcho's split-out upstream PRs. Port them one at a time, each through CI, preserving authorship (see `AGENTS.md`, "Attribution").

| # | Item | Upstream | Fixes / relates to | Risk and notes |
|---|---|---|---|---|
| 1 | Fix FPS slowdown in long sessions (64-bit timing, 32-bit unit IDs) | PR #188 | #166, #146, #141 | Small. Most-reported complaint. |
| 2 | Shrink vertex buffer (4M → 1M vertices) | PR #187 | #125 (unconfirmed) | Trivial, but a smaller buffer could overflow on heavy scenes. Check. |
| 3 | Extend support to all game versions (per-version cell/unit structs, 1.10f/1.12 resolution mod, SGD2FreeRes update) | PR #185 | #172, #167, #165, #155, #95, #38 | Large. **Do not ship a binary `SGD2FreeRes.dll` unless its source and licence are confirmed** (see Licensing in `AGENTS.md`). |
| 4 | Full-screen-width cutscenes + `nokeepaspectratio` | PR #186 | #99; related #78, #182, #173, #212 | **Conflicts with our `GetRenderRect` fix** (signature change in `Metrics.cpp`); resolve against the fix and keep the tests passing. |
| 5 | xxHash for texture/palette hashing | PR #190 | #140 (maybe #166, #146) | Risky: once broke Median XL (author says fixed); hardcoded hash constants must be updated consistently. Needs `thirdparty/xxhash` with its licence and a README credit. |
| 6 | Cursor clipping / activation fixes | PR #192 | #159; related #50, #85 | Overlaps `RenderContext.cpp` with #186 and #190; rebase last. |

## Backlog (unscheduled)
| Item | Source | Notes |
|---|---|---|
| Nearest-neighbour / pixel-exact upscaling | Jarcho fork (not in a PR) | #181. Would be lifted from the fork. |
| Float window scale (`scale=1.25`) | PR #133 | In our tree nothing outside `Options.cpp/.h` reads `GetWindowScale()`, so the parsed value may be unused. Find the consumer first. Unverified: whether `toml_double_in` rejects integer literals like `scale=2`. |
| Other cursor/window/input fixes (mouse wheel, focus loss, window position, compat-mode detection) | Jarcho fork | #127, #96, #123, #50, #108, #158, #163, #152. Cherry-pick as small commits after the planned items. |
| Multi-monitor / fullscreen, `noframetearing`, frame profiler, offscreen draw-call culling | Jarcho fork | No open issue asks for these. |
| 1.14c and older-version support (1.07–1.11) | Jarcho fork, rnd2k (1.11b) | Only if users ask. |

## Low priority / won't do
| Item | Decision | Why | Revisit if |
|---|---|---|---|
| PR #177 (Jarcho's all-in-one branch) | Won't do | Superset of #185–#188 and #192; the author says it is not stable; long thread of jitter complaints. Use the split PRs. | — |
| Adopt Jarcho's fork wholesale / external `d2fps.dll` motion prediction | Won't do now | +12k/−7k diff, deletes the in-tree predictors, ships a separate DLL. Too big a change to review as one step. | Cherry-picking the PRs proves impractical. |
| rnd2k's module-based version-detection rewrite | Won't do | Diverges heavily (`GameHelper.cpp` +~750 lines); regression risk for 1.13/1.14 unverified. | We need 1.11b or detection fixes (#163, #164). |
| rnd2k's `LoadGammaTable` stub and `D2ExpRes.dll` patching (`DllNotify`) | Won't do | Disables gamma; patches third-party software. | — |
| MarcosT96's LATAM SGD2FreeRes build pipeline | Won't do | Specific to one private server; builds from an external mutable branch. | — |
| Viewport scale option (`game.scale`) | Low priority | PR #129: hacky cursor-clip math, conflicts with #186 and #192. | Someone needs custom viewport scaling. |
| Xenthalon's `-nowindowmove` | Low priority | Tiny, matched to #108/#50 only by title. | Window-position issues remain after #192. |
| The other ~47 forks | Ignore | Dormant copies with no unique commits. | — |

## Reference: forks reviewed (2026-10-24)
| Fork | Ahead of upstream | Purpose |
|---|---|---|
| Jarcho/d2dx | 97 | Maintained continuation (v0.100–v0.103.4, RELEASES.md); source of PRs #133 (shared), #177, #185–#188, #190, #192 |
| rnd2k/d2dx_eleriaqueen | 29 | Community build merging Jarcho's and nyukhalov's work plus 1.11b support and hacks |
| CaptainRay1993/d2dx (branch `PR-Merge`) | 3 | Personal build merging #185/#186/#190/#133; conflicts with our `Metrics.cpp` |
| nyukhalov/d2dx | 3 | Float window scale (= PR #133) |
| MarcosT96/d2dx | 5 | CI for a LATAM server's SGD2FreeRes build |
| Fragonite/scaling, Xenthalon/option-noWindowMove | 1, 2 | PR #129 (viewport scale); `-nowindowmove` |
