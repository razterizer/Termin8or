# Release Notes

## 2.1.0.3

**Texture Format & Versioning**
- Added automatic minimal-version inference via `compute_minimal_version()`.
- Internal version representation standardized (`VER 1.0 → 10` using `maj*10 + min` scheme).
- Save operation now always writes an explicit `VER x.y` header.
- Texture format auto-promotes to:
  - `VER 2.0` when 8-bit colors are used.
  - `VER 2.1` when materials contain `-1`.
- Ensured full round-trip consistency between `load()` and `save()`.
  
**Texture System**
- Added support for `-1` materials in load/save.
- Fixed `subset()` to correctly copy characters, colors, and materials.
  
**UI**
- Added `Dialog::set_textel_str_pre()` for more flexible textel pre-styling.
  
**Examples**
- Reordered example execution in `examples.cpp` and removed header prints.

## 2.0.0.2
- Added support for full 256 (8-bit) Colors (actually 259 colors with default color and two transparency colors/modes).
- Renamed enum class Color -> Color16.
- Added new Color struct that can represent all 8-bit colors by constructing it from either Color16 struct, RGB6 object (new), rgb6 int triplet, Gray24 object (new) or just a color index.
- Replacing a lot of Color16 (formerly Color) with new Color struct to make the code much more general.
- Texture now supports 256 (8-bit) Colors (for `*.tx` version with header line VER 2).
- Updated the ColorPicker widget to support all 8-bit colors and allows you to pick and mix four different color regions: special colors, 4-bit, rgb6 and gray24.
- Upgraded ScreenScaling to support the new 8-bit color space.
- On Windows: When detecting that classic cmd is in use, then all colors are downsampled to a Color16 enum by a closest-distance-to-corresponding-rgb-value heuristic.
- Adding an example (Texture_examples.h) which showcases all 256+3 colors available.
- Adding an example (ScreenScaling_examples.h) which showcases upsampling of a texture.

## 1.0.1.1
- tag_release.h: Now ouputs version.h in a folder version/. Currently untested.
- examples.cpp: Adding CL args so you can play with the examples a little.
- examples.cpp: Adding an info screen so you know how to skip to the next example and eventually quit.

## 1.0.0.0
- First release.

