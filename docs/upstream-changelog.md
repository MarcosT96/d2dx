# Upstream release history (bolrog/d2dx 0.99.x)

This is the release history from the README of the original project, D2DX by bolrog (GPLv3). It ends at 0.99.529 (May 2021); later upstream commits were never released. This fork's own releases are in [CHANGELOG.md](https://github.com/jackrayner/d2dx/blob/main/CHANGELOG.md).

## 0.99.529
  - Add motion prediction for 1.09d, complete except for hovering text (it's coming).
  - Fix low fps in the menus for 1.09d.

## 0.99.527b
  - Add 'filtering' option in cfg file, which allows using bilinear filtering or Catmull-Rom for scaling the game, 
    for those who prefer the softer look over the default integer-scale/sharp-bilinear.
  - Fix artifacts when vsync is off.

## 0.99.526b
  - Fix motion-predicted texts looking corrupted/being positioned wrong.

## 0.99.525
  - Fix motion prediction of shadows not working for some units.
  - Fix window size when window is scaled beyond the desktop dimensions.
  - Fix some black text in old versions of MedianXL.
  - Remove -dxtestsleepfix, this is now enabled by default (along with the fps fix).

## 0.99.521
  - Fix low fps in menu for 1.13d with basemod.
  - Fix low fps for 1.13c and 1.14 with basemod.
  - Fix basemod compatibility so that "BypassFPS" can be enabled without ill effects.

## 0.99.519
  - Unlock FPS in menus. (Known issue: char select screen animates too fast)
  - Add experimental "sleep fix" to reduce microstutter in-game. Can be enabled with -dxtestsleepfix.
    Let me know if you notice any improvements to overall smoothness in-game, or any problems!

## 0.99.518b
  - Fix size of window being larger than desktop area with -dxscaleN.

## 0.99.517
  - Fix in-game state detection, causing DX logo to show in-game and other issues.

## 0.99.516
  - High FPS (motion prediction) is now default enabled on supported game versions (1.12, 1.13c, 1.13d and 1.14d).
  - Fix crash when trying to host TCP/IP game.

## 0.99.512c
  - Add "frameless" window option in cfg file, for hiding the window frame.
  - Fix corrupt graphics in low lighting detail mode.
  - Fix corrupt graphics in perspective mode.
  - Fix distorted automap cross.
  - Fix mouse sometimes getting stuck on the edge of the screen when setting a custom resolution in the cfg file.

## 0.99.511
  - Change resolution mod from D2HD to the newer SGD2FreeRes (both by Mir Drualga).
    Custom resolutions now work in 1.09 and 1.14d, but (at this time) there is no support for 1.12. Let me know if this is a problem!
  - Some performance optimizations.
  - Remove sizing handles on the window (this was never intended).

## 0.99.510
  - Add the possibility to set a custom in-game resolution in d2dx.cfg. See the wiki for details.
  - Remove special case for MedianXL Sigma (don't limit to 1024x768).

## 0.99.508
  - Fix motion prediction of water splashes e.g. in Kurast Docks.

## 0.99.507
  - Add motion prediction to weather.
  - Improve visual quality of weather a bit (currently only with -dxtestmop).
  - Don't block Project Diablo 2 when detected.

## 0.99.506
  - Fix crash sometimes happening when using a town portal.
  - Add motion prediction to missiles.

## 0.99.505b
  - Fix bug causing crash when using config file.
  - Add option to set window position in config file. (default is to center)
  - Update: Fix tracking of belt items and auras.
  - Update: Fix teleportation causing "drift" in motion prediction.

## 0.99.504
  - Revamp configuration file support. NOTE that the old d2dx.cfg format has changed! See d2dx-defaults.cfg for instructions.

## 0.99.503
  - Add -dxnotitlechange option to leave window title alone. [patch by Xenthalon]
  - Fix -dxscale2/3 not being applied correctly. [patch by Xenthalon]
  - Improve the WIP -dxtestmop mode. Now handles movement of all units, not just the player.

## 0.99.430b
  - Add experimental motion prediction ("smooth movement") feature. This gives actual in-game fps above 25. It is a work in progress, see
    the Wiki page (https://github.com/bolrog/d2dx/wiki/Motion-Prediction) for info on how to enable it.
  - Updated: fix some glitches.

## 0.99.429
  - Fix AA being applied on merc portraits, and on text (again).

## 0.99.428
  - Fix AA sometimes being applied to the interior of text.

## 0.99.423b
  - Fix high CPU usage.
  - Improve caching.
  - Remove registry fix (no longer needed).
  - Updated: Fix AA being applied to some UI elements (it should not).
  - Updated: Fix d2dx logo.

## 0.99.422
  - Fix missing stars in Arcane Sanctuary.
  - Fix AA behind some transparent effects, e.g. character behind aura fx.
  - Add -dxnocompatmodefix command line option (can be used in cases where other mods require XP compat mode).

## 0.99.419
  - Fix issue where "tooltip" didn't pop up immediately after placing an item in the inventory.
  - Add support for cfg file (named d2dx.cfg, should contain cmdline args including '-').
  - Further limit where AA can possibly be applied (only on relevant edges in-game and exclude UI surfaces).
  - Performance optimizations.

## 0.99.415
  - Add fps fix that greatly smoothes out frame pacing and mouse cursor movement. Can be disabled with -dxnofpsfix.
  - Improve color precision a bit on devices that support 10 bits per channel (this is not HDR, just reduces precision loss from in-game lighting/gamma).
  - To improve bug reports, a log file (d2dx_log.txt) will be written to the Diablo II folder.

## 0.99.414
  - The mouse cursor is now confined to the game window by default. Disable with -dxnoclipcursor.
  - Finish AA implementation, giving improved quality.
  - Reverted behavior: the mouse cursor will now "jump" when opening inventory like in the unmodded game. This avoids the character suddenly changing movement direction if holding LMB while opening the inventory.
  - Fix issue where D2DX was not able to turn off XP (etc) compatibility mode.
  - Fix issue where D2DX used the DX 10.1 feature level by default even on better GPUs.
  - Fix misc bugs.

## 0.99.413
  - Turn off XP compatibility mode for the game executables. It is not necessary with D2DX and causes issues like graphics corruption.
  - Fix initial window size erroneously being 640x480 during intro FMVs.

## 0.99.412
  - Added (tasteful) FXAA, which is enabled by default since it looked so nice. (It doesn't destroy any detail.)

## 0.99.411
  - D2DX should now work on DirectX 10/10.1 graphics cards.

## 0.99.410
  - Improved non-integer scaling quality, using "anti-aliased nearest" sampling (similar to sharp-bilinear).
  - Specifying -dxnowide will now select a custom screen mode that gives integer scaling, but with ~4:3 aspect ratio.
  - Added -dxnoresmod option, which turns off the built-in SlashDiablo-HD (no custom resolutions).
  - (For res mod authors) Tuned configurator API to disable the built-in SlashDiablo-HD automatically when used.
  - Other internal improvements.

## 0.99.408
  - Fix window size being squashed vertically after alt-entering from fullscreen mode.
  - Fix crash when running on Windows 7.

A full release history can be found on the Wiki.
