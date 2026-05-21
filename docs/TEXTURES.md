# Textures

## Overview

The `Texture` struct can be found in `include/Termin8or/drawing/Texture.h` under the `t8` namespace.

Textures can be used for various purposes:
- Sprites.
- Offscreen buffer rendering (screen to texture).
- Animated level/map textures for dungeon decoration in dungeon engine [DungGine](https://github.com/razterizer/DungGine).
- TextUR image storage.

## Texture Data Model

`Texture` stores cells in a 2D grid layout as flat "SoA" vectors which can be indexed by row and column coordinates using the function `inline int Texture::index(int r, int c) const noexcept`. Data in the flat vectors are organized by row-major order (i.e. line by line).

A `Texture` stores:
- `size`
- `area`
- `glyphs` per cell.
- `fg_colors` per cell.
- `bg_colors` per cell.
- `materials_raw` per cell (material ID, `t8::texture::raw_mat_none = 0xFF`).

The data elements for a particular cell can be accessed as a bundle by using the operator `Textel Texture::operator()(int r, int c) const` where `Textel` (textel here is basically another name for cell in the context of `Texture` objects) is a representation of the data like glyph, fg/bg colors and material of a cell in the texture object.

The number of bytes (excluding padding) stored by a single textel (texture cell) can be determined by the types of the textel data:
* `Glyph glyph` : `char32_t` (4) + `char` (1).
* `Color fg_color` : `int16_t` (2).
* `Color bg_color` : `int16_t` (2).
* `uint8_t raw_mat` : `uint8_t` (1).

So every textel is `(4 + 1) + 2*2 + 1 = 10 bytes`.

## Materials

Materials are stored as `uint8_t` bytes in order to utilize the full positive range of the byte. In order to have a marker for "no material" sentinel `t8::texture::raw_mat_none = 0xFF` which means you have 255 available materials excluding this sentinel constant.

A raw material is `raw_mat_none` by default.

Raw materials are not used externally though. Instead, to make it easier to use the API there is a public layer `material` where sentinel `t8::texture::mat_none = -1` is used. This sentinel directly translates to `raw_mat_none`. A material is `int` (4 bytes for most compilers/platforms ) which means that when encoding it to a raw material, higher values than 254 are clamped to 254.

The (raw and public) material range is thus in the range `[0, 254]`.

You can use the following functions in `Texture.h` for conversion between these domains:

* `inline uint8_t t8::texture::encode_raw_material(int m)`.
* `inline int t8::texture::decode_raw_material(uint8_t m_raw)`.

So prefer to use public (decoded) API functions unless you specifically need raw data manipulation.

Example: `t8::texture::encode_raw_material(-1) == t8::texture::raw_mat_none // 0xFF (255)`.

Materials are mainly used for collision surface generation, but additional uses can be found.

Other useful helpers in `Texture.h` are:

* `inline bool t8::texture::has_raw_material(uint8_t m_raw)`.
* `inline bool t8::texture::has_material(int m)`.

## Creating Textures

Examples:
- empty/default texture
- fixed-size texture
- fill with glyph/style
- set individual textel/glyph/color/material

Keep examples short.

## Loading and Saving

Explain current public API:
- `TextureFile::load(...)` / `TextureFile::save(...)`
- `TextureFileFormat::Auto`
- format-specific options for TX / ANSI

Mention:
- `.tx` is native format
- ANSI formats are supported but lossy/sidecar-aware depending on encoding/material/fallbacks

Link:
- `TX_FORMAT.md`
- `ANSI.md`

## Drawing Textures

Explain that textures can be rendered through:
- `ScreenHandler::write_buffer(...)`
- drawing helpers like `draw_box_textured(...)`
- sprites / sprite frames

## Unicode and Fallbacks

Short cross-link to `GLYPHS.md`:
- textures store `Glyph`, not plain `char`
- Unicode glyphs need ASCII fallback
- ASCII-only rendering uses fallback

## Texture File Formats

Short summary only:
- `.tx`: native Termin8or texture format
- `.ans/.ansi/.diz/.txt`: ANSI/CP437 style
- `.utf8ans`: UTF-8 ANSI style

Then link to format docs.

## Compatibility Notes

Mention:
- Older texture files may be ASCII-only.
- Newer texture files may use bracketed glyphs.
- Materials changed to raw storage internally but decoded public API should be preferred.
