# Configuring D2DX

Written from the source (`src/d2dx/Options.cpp`) and `d2dx-defaults.cfg`. The original project's [wiki](https://github.com/bolrog/d2dx/wiki/) has more background but describes upstream and may be out of date for this fork.

## Config file

Copy `d2dx-defaults.cfg` from the release zip, rename it to `d2dx.cfg`, edit it, and put it in your Diablo II folder. It is [TOML](https://toml.io). Every setting is optional; anything you leave out keeps its default. Settings given on the command line (below) are applied after the file.

### `[window]`
| Key | Default | Meaning |
|---|---|---|
| `scale` | `1` | Integer scale factor for the window, 1 to 3. |
| `position` | `[-1,-1]` | Window position as `[x,y]`. `[-1,-1]` centres the window. |
| `frameless` | `false` | Remove the window frame (caption bar etc). |

### `[game]`
| Key | Default | Meaning |
|---|---|---|
| `size` | `[-1,-1]` | In-game resolution as `[width,height]`. `[-1,-1]` lets D2DX choose a suitable size. |
| `filtering` | `0` | Scaling filter: `0` high quality (sharp, more pixelated), `1` bilinear (blurry), `2` Catmull-Rom (higher quality than bilinear). |

Custom sizes need the built-in resolution mod, which only runs on some game versions (see [compatibility](https://github.com/jackrayner/d2dx/blob/main/docs/compatibility.md)).

### `[optouts]`
All default to `false`. Setting one to `true` turns the matching D2DX behaviour off.

| Key | Effect when `true` |
|---|---|
| `noclipcursor` | Don't lock the mouse cursor to the game window. |
| `nofpsfix` | Don't apply the basic FPS fix (this also rules out high FPS). |
| `noresmod` | Don't use the built-in resolution mod (this rules out widescreen). |
| `nowide` | Don't choose a widescreen resolution (does nothing if `noresmod` is `true`). |
| `nologo` | Don't show the D2DX logo on the title screen. |
| `novsync` | Don't use vertical sync. |
| `noaa` | Don't apply anti-aliasing to jagged edges. |
| `nocompatmodefix` | Don't block Windows XP compatibility mode. Needed under Wine/Proton: Wine reports Windows 6.2 to one API and 10.0 to another, which D2DX would otherwise take for compatibility mode. (This is from a report of a 1.14d run under Proton, not from reading the Wine source.) |
| `notitlechange` | Don't change the window title. |
| `nomotionprediction` | Don't run the game graphics at high FPS. |

### `[debug]`
| Key | Default | Meaning |
|---|---|---|
| `dumptextures` | `false` | Write every texture D2DX uploads to `dump/<category>/<hash>.bmp` in the game folder. For debugging only; it writes a lot of files. |

## Command line

Options are matched anywhere in the command line. Besides the game's own `-3dfx` (required to use D2DX) and `-w` (windowed):

| Option | Same as |
|---|---|
| `-dxnoclipcursor`, `-dxnofpsfix`, `-dxnoresmod`, `-dxnowide`, `-dxnologo`, `-dxnovsync`, `-dxnoaa`, `-dxnocompatmodefix`, `-dxnotitlechange` | the `[optouts]` key of the same name, set to `true` |
| `-dxnomop` | `nomotionprediction = true` |
| `-dxscale2`, `-dxscale3` | `[window] scale = 2` or `3` |
| `-dxdbg_dump_textures` | `[debug] dumptextures = true` |

## Log file

D2DX writes `d2dx_log.txt` in the game folder at startup. Attach it when reporting a problem; it shows the detected game version, which features were enabled, and the render size.
