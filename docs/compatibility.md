# Compatibility

## Game versions

Derived from the source: which versions get the built-in resolution mod is decided in `BuiltinResMod.cpp`, and which get motion prediction (the high-FPS feature) in `D2DXContext.cpp`. "Tested" means somebody has actually played it with this fork's build. CI builds the DLL and runs unit tests but cannot run the game.

| Game version | Custom resolutions | High FPS (motion prediction) | Tested with this fork |
|---|---|---|---|
| 1.09d | yes | units, weather | no |
| 1.10f | no | no | no |
| 1.12 | no | units, weather | no |
| 1.13c | yes | units, weather, text | no |
| 1.13d | yes | units, weather, text | no |
| 1.14d | yes | units, weather, text | yes, under Wine/Proton (0.0.1) |

Other versions are not supported. The rest of D2DX (scaling, anti-aliasing, windowed/fullscreen handling) is not limited by version in the same way, but only 1.14d has been tried.

Please report results for the untested versions through the [issue tracker](https://github.com/jackrayner/d2dx/issues) (include `d2dx_log.txt`).

## Windows and Wine

- **Windows 10** is recommended. Upstream supported Windows 7 SP1 and later; this fork's DLL is built with the newer v143 toolset and has not been tried on Windows 7 or 8.1.
- **Linux (Wine/Proton):** 0.0.1 was played on 1.14d through Lutris with Proton on Fedora and ran fine, using `nocompatmodefix=true` in `d2dx.cfg` (see [configuration](https://github.com/jackrayner/d2dx/blob/main/docs/configuration.md)). That is one report on one setup.
- The DLL is statically linked and uses only Windows system DLLs, so no Visual C++ runtime install should be needed.

## Mods

Other mods change the game files and addresses D2DX hooks, so results vary. Upstream kept a list on its [wiki](https://github.com/bolrog/d2dx/wiki/Compatibility-with-other-mods); it describes upstream's builds. Project Diablo 2 is detected and has its own handling of the suggested resolution.
