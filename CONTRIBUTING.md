# Contributing

Thanks for helping. This is a maintained fork of [bolrog/d2dx](https://github.com/bolrog/d2dx) (see the README). Planned and declined work is in [DEVELOPMENT.md](DEVELOPMENT.md); check it before starting something big. `AGENTS.md` has more detail on the code layout, written for coding agents but useful to anyone.

## Build

D2DX is a Win32 (x86) Visual C++ project: open `src/d2dx.sln` in Visual Studio and build `Release|Win32` (the project targets the v142 toolset; CI retargets to v143). The `d2dx` project's post-build step copies `glide3x.dll` to the original author's game folder (`C:\games\Diablo II\`); change that locally, don't commit it.

## Test

- **Windows CI** (`.github/workflows/tests.yml`) builds the DLL and the test project and runs the unit tests on every pull request. Docs-only changes skip the Windows job.
- **Locally on Linux or macOS:** `tests/linux/run.sh` runs the pure-logic tests in a container (podman or docker). It covers logic only, not MSVC behaviour or the game.
- Nothing automated plays the game. If you change rendering, hooks or game-version handling, run the DLL in a Diablo II folder (`-3dfx`) and say in the pull request what you tried.

## Pull requests

- Branch from `main` and open a pull request. The `build-and-test` check must pass.
- Use **conventional commit titles** (`fix:`, `feat:`, `perf:`, `docs:`, `build:`, `ci:`, `chore:`). Releases are automated from them; see "Releasing" in `AGENTS.md`. Squash merges work best.
- Add or update unit tests for logic that doesn't need the game (`src/d2dxtests`).
- Port other people's work with `git cherry-pick -x` and keep their authorship and licence headers (GPLv3, see `AGENTS.md`, "Licensing and attribution").
- Don't write issue or pull request numbers or links from other people's repositories in commit messages or PR text: GitHub posts a "mentioned in" note on their thread that can't be removed.

## Reporting problems

Open an [issue](https://github.com/jackrayner/d2dx/issues) with your game version, OS (and Wine/Proton if used), and the `d2dx_log.txt` from your Diablo II folder.
