# Release Notes

## 3.0.2.8

- Added support for [forge](https://github.com/razterizer/forge).
- Smoked out a lot of include and forward declare misses due to new cbox release build workflow job.

## 3.0.1.7

**ANSI / CP437 Import & Export**
- Added ANSI-art load/save support for `.asc` and `.nfo` files.
- Added unit test for auto-detection of ansi file extensions.

## 3.0.0.6

This release is a major Unicode/API upgrade for Termin8or. The core rendering model now supports Unicode-capable glyphs with strict ASCII fallbacks, while preserving the older ASCII-only path for simpler terminals and deterministic output.

**Glyphs & Strings**
- Added `t8::Glyph`, a small glyph object with a preferred `char32_t` code point and a printable ASCII fallback.
- Added strict glyph invariants so invalid fallback states, non-printable ASCII preferred glyphs, and missing fallbacks are caught early.
- Added `t8::GlyphString` for glyph-aware single-line strings.
- Replaced ambiguous `GlyphString(std::string)` / `GlyphString(std::string_view)` construction with explicit factories such as `from_ascii()`, `from_utf8()`, and `from_number()`.
- Added the `_gs` literal for ergonomic ASCII `GlyphString` construction.
- Added tests for glyph-string construction, UTF-8 handling, and related edge cases.

**Unicode Rendering**
- Added Unicode-aware rendering support through `ScreenHandler<CharT>`, with `char` remaining ASCII-only and `char32_t` enabling preferred Unicode glyph output.
- Added runtime ASCII fallback policy support via `AsciiFallbackPolicy`.
- Added terminal/font-aware glyph coverage checks where possible, while keeping safe ASCII fallback behavior for unsupported glyphs.
- Added Core-backed helpers for ASCII and printable-ASCII checks.

**Textures**
- Updated `Texture` / `Textel` handling to store glyphs rather than raw chars.
- Separated raw stored texture materials from decoded material IDs:
  - `raw_mat_none = 255`
  - `mat_none = -1`
  - added helpers for raw/decoded material conversion and checks.
- Updated drawing, sprite, dynamics, and rigid-body code to use the clarified material semantics.
- Fixed sprite line-segment default glyph initialization that could create invalid glyphs.

**Texture File I/O**
- Moved texture file loading/saving out of `Texture.h` into dedicated texture-file headers.
- Added `TextureFile`, `TextureFileTx`, `TextureFileAnsi`, and common texture-file helpers.
- Added format selection through `TextureFileFormat`.
- Added new `.tx` format support for Unicode glyph serialization:
  - `VER 3.0` supports bracketed glyph serialization with preferred Unicode code point and ASCII fallback.
  - default `.tx` saving now uses a Unicode-preserving mode when needed.
- Added texture file unit tests covering material conversion and `.tx` roundtrips.

**ANSI / CP437 Import & Export**
- Added ANSI-family load/save support for `.ans`, `.ansi`, `.diz`, `.txt`, and `.utf8ans`.
- Added CP437 and UTF-8 ANSI loading, including UTF-8 BOM handling.
- Added automatic load encoding selection:
  - UTF-8 for BOM / `.utf8ans`.
  - CP437 for ordinary ANSI files by default.
- Added ANSI save encoding modes for preserving glyphs, preferring CP437, or forcing UTF-8/CP437.
- Added CP437 Unicode conversion support, including low-byte graphical glyphs.
- Added built-in ASCII fallbacks for common CP437/Unicode ANSI-art glyphs.
- Added sidecar support for ANSI fallbacks (`.fb`) and materials (`.mat`).
- Added support for a practical subset of ANSI SGR, cursor movement, save/restore cursor, erase, wrapping, DOS EOF, and SAUCE-related payload trimming.

**UI & Widgets**
- Updated glyph picker workflows for Unicode preferred glyphs and ASCII fallbacks.
- Added fallback hex entry support for glyphs that are awkward to type directly.
- Updated text box, string box, dialog, and message handling paths for stricter `GlyphString` semantics.
- Added multiline message support for glyph-string based message handlers.
- Added several dialog utility functions for clearing or querying picker/text-field state.

**Engine & System**
- Improved `GameEngine` lifecycle handling so `run()` owns initialization and data generation.
- Added `GameEngine::set_allow_quitting()` so applications can block quitting while editing modal text input.
- Improved keyboard/raw-mode and logging behavior by returning failure states instead of exiting abruptly.

**Documentation**
- Refreshed the main README.
- Added dedicated documentation:
  - `docs/GLYPHS.md`
  - `docs/TEXTURES.md`
  - `docs/TX_FORMAT.md`
  - `docs/ANSI.md`
  - `docs/MIGRATION_UTF8.md`

**Breaking / Migration Notes**
- Glyph fallbacks must be printable ASCII.
- Non-printable ASCII code points are not valid preferred glyphs.
- Texture material APIs now distinguish decoded material IDs from raw stored material bytes.
- Older tools that only understand `.tx` versions up to `VER 2.1` will not understand Unicode glyph data saved as `VER 3.0`.

## 2.1.2.5
- Fixed lagging version in released version.h issue.

## 2.1.1.4
- `Textel::mat_to_char_str()` now supports `mat = -1` -> `"-"`.

## 2.1.0.3

**Texture Format & Versioning**
- Added automatic minimal-version inference via `compute_minimal_version()`.
- Internal version representation standardized (`VER 1.0 → 10` using `maj*10 + min` scheme).
- Save operation now always writes an explicit `VER x.y` header.
- Texture format auto-promotes to:
  - `VER 2.0` when 8-bit colors are used.
  - `VER 2.1` when materials contain `-1`.
- Ensured full round-trip consistency between `load()` and `save()`.
- Fixed format bug in save() where long Color string format was used rather than the compact format.
  
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
