# D2DX

D2DX is a Glide-wrapper and mod that makes the classic Diablo II/LoD run well on modern PCs, while honoring the original look and feel of the game.
Play in a window or in fullscreen, glitch-free, with or without enhancements like widescreen, true high framerate and anti-aliasing.

Version 0.0.1 <!-- x-release-please-version -->

This is a maintained fork of [bolrog/d2dx](https://github.com/bolrog/d2dx), which has not been updated since July 2021. It is not intended to be merged back upstream. Planned and declined work is tracked in [DEVELOPMENT.md](https://github.com/jackrayner/d2dx/blob/main/DEVELOPMENT.md).

## Features
  - Turns the game into a well behaved DirectX 11 title on Windows 10 (also 7, 8 and 8.1).
  - High quality scaling to fit modern screen sizes, including [widescreen aspect ratios](https://raw.githubusercontent.com/bolrog/d2dx/main/screenshots/d2dx2.png).
  - High FPS mod using motion prediction, bypassing the internal 25 fps limit. Play at 60 fps and higher! ([video](https://imgur.com/a/J1F8Ctb))
  - [Anti-aliasing](https://github.com/bolrog/d2dx/wiki/Screenshots#anti-aliasing) of specific jagged edges in the game (sprites, walls, some floors).
  - Seamless windowed/fullscreen switching with (ALT-Enter).
  - Improved fullscreen: instant ALT-TAB and low latency.
  - Improved windowed mode.
  - Proper gamma/contrast.
  - Fixes a few window-related glitches in Diablo II itself.

### Video Showcasing Motion Prediction
  [FPS increase in menus, FPS increase for projectiles, monsters, +more](https://imgur.com/a/J1F8Ctb)

## Requirements
  - Diablo 2: LoD (see Compatibility section below).
  - Windows 10 recommended (for latency improvements). Upstream supported Windows 7 SP1 and above, but this fork's builds have not been tried on Windows 7 or 8.1.
  - A CPU with SSE2 support.
  - Integrated graphics or discrete GPU with DirectX 10.1 support.

## Compatibility
Game versions supported:
  - All features: 1.09d, 1.13c, 1.13d and 1.14d.
  - Without resolution switching: 1.10f, 1.12.
  - Other versions are unsupported, will display a warning at startup and exhibit glitches.

For compatibility with mods, see the [wiki](https://github.com/bolrog/d2dx/wiki/Compatibility-with-other-mods).

## Documentation
  This readme contains basic information to get you started. [`d2dx-defaults.cfg`](https://github.com/jackrayner/d2dx/blob/main/d2dx-defaults.cfg) documents the settings. More is on the original project's [wiki](https://github.com/bolrog/d2dx/wiki/), which describes upstream and may be out of date for this fork.

## Installation
  Download `d2dx.zip` from the [latest release](https://github.com/jackrayner/d2dx/releases/latest) and copy `glide3x.dll` into your Diablo II folder.

  No Visual C++ runtime install should be needed: the DLL is statically linked and only uses Windows system DLLs.

## Usage
To start the game with D2DX enabled, just provide -3dfx, e.g.
  ```
  Game.exe -3dfx
  ```
Windowed/fullscreen mode can be switched at any time by pressing ALT-Enter. The normal -w command-line option works too.

Many of the default settings of D2DX can be changed. To use a configuration file, rename `d2dx-defaults.cfg` to `d2dx.cfg` and edit it. A list of command-line options is on the original project's [wiki](https://github.com/bolrog/d2dx/wiki/).

## Troubleshooting

### I get a message box saying "Diablo II is unable to proceed. Unsupported graphics mode."
  You are running the download version of Diablo II from blizzard.com. It can be modified to work with D2DX, but this fork does not document how yet.

### It's ugly/slow/buggy.
  Check [DEVELOPMENT.md](https://github.com/jackrayner/d2dx/blob/main/DEVELOPMENT.md) to see whether it is a known or planned item, and include `d2dx_log.txt` from your Diablo II folder when reporting a problem.

## Credits
Original author: bolrog
Patch contributions: Xenthalon
Fork maintenance: Jack Rayner

The research of many people in the Diablo II community over twenty years made this project possible.

Thanks to Mir Drualga for making the fantastic SGD2FreeRes mod!
Thanks also to everyone who contributes bug reports.

D2DX uses the following third party libraries:
- FNV1a hash reference implementation, which is in the public domain.
- Detours by Microsoft.
- SGD2FreeRes by Mir Drualga, licensed under Affero GPL v3.
- FXAA implementation by Timothy Lottes. (This software contains source code provided by NVIDIA Corporation.)
- stb_image by Sean Barrett
- pocketlzma by Robin Berg Pettersen
- tomlc99 by CK Tan, MIT licensed.
- 9-tap Catmull-Rom texture filtering by TheRealMJP.

## Release history
This fork's releases are in [CHANGELOG.md](https://github.com/jackrayner/d2dx/blob/main/CHANGELOG.md). The original project's history up to 0.99.529 is in [docs/upstream-changelog.md](https://github.com/jackrayner/d2dx/blob/main/docs/upstream-changelog.md).
