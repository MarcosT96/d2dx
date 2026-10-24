# AGENTS.md

Guidance for AI coding agents (and humans) working in this repo. `CLAUDE.md` just imports this file.

## What this is

D2DX is a **Glide 3x wrapper DLL** for Diablo II: Lord of Destruction. It is built as `glide3x.dll`, dropped into the game folder, and loaded when the game runs with `-3dfx`. It translates the game's Glide calls to Direct3D 11, and uses function hooks (Microsoft Detours) to add widescreen, scaling, anti-aliasing and high-FPS motion prediction.

- **This repo is a fork** (`origin` = `jackrayner/d2dx`) of [bolrog/d2dx](https://github.com/bolrog/d2dx). Upstream history is preserved; the last upstream commit was July 2021 (`d95d19a`).
- **`main` is ahead of the last release.** Release `0.99.529` (tag `6929e4f`, 2021-05-29) is 10 commits behind. Four of those change code: three 1.10f fixes (`aa5d689`, `d937337`, `78d2776`) and the Project Diablo 2 suggested-resolution fix (`c04ddaa`). The README and `.rc` still say 0.99.529, so a binary built from `main` is an unreleased build. README claims about game support may predate those fixes. Upstream is not configured as a remote; add one if you need to sync.
- License: GPLv3 (`LICENSE.txt`). Source files carry a GPL header; keep it on new files. Bundled SGD2FreeRes is AGPLv3, see `thirdparty/`.
- `README.md` is user-facing and still describes upstream (credits, wiki links, changelog). Don't treat it as fork-specific documentation.

## Build and test: Windows only

- Visual Studio 2019 solution: `src/d2dx.sln`. Toolset `v142`, C++ `stdcpplatest`, **Win32 (x86) only**, Debug/Release. The game is 32-bit, so the DLL must be x86.
- Projects: `src/d2dx` (the DLL) and `src/d2dxtests` (Microsoft CppUnitTest, run from VS Test Explorer).
- HLSL shaders (`*.hlsl`, `*.hlsli`) are compiled by MSBuild during the build into generated headers (`*_cso.h`, gitignored). `Display.hlsli`/`Game.hlsli`/`Constants.hlsli`/`FXAA.hlsli` are shared includes.
- The `d2dx` post-build step copies `glide3x.dll` to `C:\games\Diablo II\`. That path is the original author's; change it locally (or ignore the failure), don't commit machine-specific paths.
- `src/d2dx/d2dx.vcxproj.user` and `src/d2dxtests/*.vcxproj.user` are tracked; avoid committing personal VS settings changes.
- **The full build only works on Windows** (Win32, D3D11, `windows.h`, MSVC SAL annotations). The dev machines here are Linux/macOS, so don't claim a change is built or tested unless one of the two test paths below actually ran it, and say which.

## Running the tests

Two paths, same test sources (`src/d2dxtests`):

1. **Portable local check (fast, partial):** `tests/linux/run.sh`. It builds a small Debian image (clang, 32-bit multilib, cmake) with podman or docker and runs the tests inside. It works the same on Fedora and macOS and installs nothing on the host. Always `linux/amd64` (emulated on Apple Silicon) because `SimdSse2.cpp` uses SSE2 intrinsics.
   - It compiles only the pure-logic sources: `SimdSse2`, `Metrics`, `TextureCache`, `TextureCachePolicyBitPmru`, plus the tests for `Batch`. Windows and D3D bits are replaced by shims in `tests/linux/shim/` (`pch.h`, `CppUnitTest.h`, `UtilsStub.cpp`, `Utils.h`, `D2DXContext.h`, `main.cpp`). Sources are copied into the build dir so the shim `pch.h` shadows the real one.
   - It is 32-bit (`-m32`) because `D2Types.h` static-asserts Win32 struct sizes.
   - If a test needs a new Windows symbol, extend the shim rather than editing production code.
   - It tests logic only, not MSVC behaviour. A pass here is not a pass on Windows.
2. **Windows CI (authoritative):** `.github/workflows/tests.yml` builds the test project and the DLL on `windows-latest` and runs `vstest.console`. It runs on PRs and on pushes to `main` (not on every branch push, so open a PR to get CI), but the Windows job is skipped when a change touches only docs (`*.md`, licences, `screenshots/`, `dependabot.yml`); a cheap `changes` job decides, and the skipped job still satisfies the required `build-and-test` check. Anything else, including `tests/linux/known-failures.txt` and the workflow itself, runs the full build. Check it with `gh run list` / `gh run view --log-failed` once `gh` is authenticated, and get it green before merging.

Rendering, Detours hooks and anything touching a live game or D3D device can't be covered by either; they need a manual run on Windows.

### Known failing tests
`tests/linux/known-failures.txt` lists tests that are expected to fail. Both runners skip them (Linux reports `[KNOWN]`, CI filters by name), so any other failure is a regression. It is currently empty. Remove an entry once its bug is fixed, and don't add entries to hide new failures. A failed `assert()` in a Debug test hangs the Windows runner (headless dialog), which is why the CI job has a 15-minute timeout.

## Architecture (src/d2dx)

Flow: `dllmain.cpp` (`DLL_PROCESS_ATTACH` → `AttachDetours()`) → game calls exported Glide functions in `glide3x.cpp` → forwarded to the singleton `ID2DXContext` (created lazily by `D2DXContextFactory`) → batches are built and drawn by `RenderContext`.

| Area | Files |
|---|---|
| Glide exports (`extern "C"`, `FX_ENTRY`); thin forwarders to the context | `glide3x.cpp`, `IGlide3x.h`, `ID2DXContext.h` |
| Central state machine: handles every Glide event, batching, frame boundaries, window handling | `D2DXContext.*` |
| Hooks into game code and Win32 APIs (Sleep, SetWindowPos, D2Gfx/D2Win/D2Client draw functions, etc.). Two stages: `AttachDetours` at load, `AttachLateDetours` once game DLLs are loaded | `Detours.*`, `IWin32InterceptionHandler.h`, `ID2InterceptionHandler.h` |
| Game-version detection and **per-version memory addresses/offsets** | `GameHelper.*`, `IGameHelper.h`, `Types.h` (`GameVersion`), `D2Types.h` |
| D3D11 device, swap chain, shaders, textures, scaling | `RenderContext.*`, `RenderContextResources.*`, `IRenderContext.h`, `*.hlsl` |
| Draw batching and vertices | `Batch.h`, `Vertex.h`, `Buffer.h` |
| Texture cache (hash-based atlas, bit-PMRU eviction), surface tracking | `TextureCache*.*`, `TextureHasher.*`, `SurfaceIdTracker.*` |
| High-FPS motion prediction | `UnitMotionPredictor.*`, `TextMotionPredictor.*`, `WeatherMotionPredictor.*` |
| Embedded SGD2FreeRes (custom resolutions); extracted from LZMA resources (`d2dx.rc`, `resource.h`) at runtime | `BuiltinResMod.*`, `thirdparty/sgd2freeres/` |
| Options: command line (`-dx...`) plus `d2dx.cfg` | `Options.*`, `D2DXConfigurator.*` |
| Misc | `ISimd.h`/`SimdSse2.*` (SSE2 abstraction), `Metrics.*`, `Utils.*` (logging, helpers), `ErrorHandling.h`, `CompatibilityModeDisabler.*`, `pch.h` |

Design notes:
- Components are interface-first (`IFoo.h` + `Foo`), injected via `std::shared_ptr` in `D2DXContext`'s constructor. This is what makes the unit tests possible (e.g. `ISimd`). Follow it for new components.
- The game is single-threaded, and the context code relies on that; there is no locking.
- Error handling: `D2DX_CHECK_HR(expr)` for HRESULTs (throws `ComException`), `D2DX_FATAL_ERROR(msg)` for unrecoverable states.
- Precompiled header: every `.cpp` starts with `#include "pch.h"`.
- Style (match it): tabs, Allman braces, `namespace d2dx`, `_In_`/`_Out_` SAL annotations on parameters, `#pragma region` for interface groups, `int32_t`/`uint32_t` fixed-width types.

## Game versions: the fragile part

Supported: 1.09d, 1.10f, 1.12, 1.13c, 1.13d, 1.14d (`GameVersion` enum in `Types.h`). The version is read from `game.exe`'s file version in `GameHelper::GetGameVersion()`; anything else is `Unsupported`.

Support is implemented by hardcoded addresses, so adding or fixing a version means touching all of these:
- `GameHelper.cpp`: per-version `switch` statements (DLL/offset lookups such as the in-game-state address, the `GameAddress` return-address table around line 122, and similar).
- `Detours.cpp`: per-version hook selection in `AttachLateDetours` (e.g. `D2Client_DrawUnit_ESI_Hooked` vs `_Stack_Hooked`, weather-particle variants, 1.09d exceptions).
- `BuiltinResMod.cpp`: only 1.09d/1.13c/1.13d/1.14d get the resolution mod; 1.10f and 1.12 run without custom resolutions.
- `README.md` compatibility section.

1.10f work landed after release 0.99.529, and the README's "1.10f: without resolution switching" line predates it, so verify actual 1.10f behaviour rather than trusting the README. Commits to read for the pattern: `aa5d689` (1.10f in-game detection), `d937337` (1.10f sleep fixes), `78d2776` (enum rename). Use `git show` on them as a template before adding a version.

Also supported/detected: Project Diablo 2 (logged in `GameHelper`); its suggested resolution has special handling.

## Working conventions
- **Tracker:** `DEVELOPMENT.md` lists planned work, the backlog, and what we've decided not to do (with reasons). Check it before starting a feature and update it in the same PR that changes an item's status.

- Follow the global rules in `~/.claude/CLAUDE.md`: no commits or pushes unless asked; confirm before destructive or outward-facing actions.
- Don't edit `thirdparty/` except to deliberately update a dependency. It holds vendored binaries (`detours.lib`, `SGD2FreeRes.*`) and licensed code.
- If you change the SGD2FreeRes binaries, regenerate the matching `.lzma` files that `d2dx.rc` embeds (note the `.rc` references lowercase `sgd2freeres.*.lzma` filenames while the files on disk are `SGD2FreeRes.*.lzma`; fine on Windows, case-sensitive on Linux).
- New user-facing options: add to `OptionsFlag` (or a typed setter) in `Options.h/.cpp`, document in `d2dx-defaults.cfg`, and mention in `README.md`.
- Keep the changelog in `README.md` current for user-visible changes, and bump the version string there.
- Add unit tests in `src/d2dxtests` for logic that doesn't need a live game or D3D device (batching, SIMD, metrics, texture cache). New test `.cpp` files must be added to both `d2dxtests.vcxproj` and `tests/linux/CMakeLists.txt`. Run `tests/linux/run.sh` before committing.

## Open questions / not yet verified

- The version lives in `README.md` and `src/d2dx/d2dx.rc`. In the `.rc`, the numeric `FILEVERSION`/`PRODUCTVERSION` are stale (`0,99,330,0`, already so at the 0.99.529 tag) while the string values say `0.99.529.0`. These describe the last release, not `main`. Update all of them together when bumping.
- Fork goals are not documented yet. Add them here once decided.

## Licensing and attribution

This is GPLv3-or-later (`LICENSE.txt`; source headers say "any later version"), and upstream forks and PRs are GPLv3 too, so porting their code is allowed. This is the working practice, not legal advice:
- **Keep authorship.** Port with `git cherry-pick -x <sha>` or `git am` so the original author and upstream commit stay in history. If you re-implement by hand, add a `Co-authored-by:` trailer and name the source PR/commit in the message.
- **Keep headers.** Never strip copyright/licence headers. New files from others keep their authors' notices. Don't present others' code as ours.
- **Credit in the README.** Add contributors whose work we ship to the README "Credits" section, and list any new third-party library there (for example xxHash, BSD-2-Clause) with its licence file kept under `thirdparty/<lib>/`.
- **Mark modifications.** GPLv3 §5(a) asks for a notice that the work was modified, with a date. Keep git history intact, and keep a short changelog or "Changes from upstream" note when a release is cut.
- **Binaries need source.** Anyone we hand a built DLL must be able to get the corresponding source (§6). Don't ship a pre-built binary from another fork (notably Jarcho's custom `SGD2FreeRes.dll`, AGPLv3) unless its source and licence are confirmed.
- Only reuse code from forks that carry a licence file. All forks reviewed so far do.
