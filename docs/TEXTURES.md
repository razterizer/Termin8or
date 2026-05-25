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

Compare this with the former `Textel` before the Unicode feature was merged in into `main`:
* `char ch` : `char` (1).
* `Color16 fg_color` : `int` (often 4).
* `Color16 bg_color` : `int` (often 4).
* `int mat` : `int` (often 4).

In total 13 bytes. So therefore, the new Textel data is not only richer in representation, it is also more compact (by 3 bytes)!

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

A default constructed `Texture` is empty. To create a texture with a fixed size, pass an `RC` size to the constructor:

```cpp
t8::Texture tex { { 10, 20 } };
```

Individual cells can be edited through the textel setters:

```cpp
tex.set_textel_glyph(2, 4, t8::Glyph { U'█', '#' });
tex.set_textel_fg_color(2, 4, t8::Color16::Yellow);
tex.set_textel_bg_color(2, 4, t8::Color16::Black);
tex.set_textel_material(2, 4, 3);
```

## Loading and Saving

Texture file I/O is handled through `TextureFile` in `include/Termin8or/drawing/TextureFile.h`.

```cpp
t8::Texture tex;
t8::TextureFile::load(tex, "sprite.tx");

t8::TextureFile::save(tex, "sprite.tx");
```

When saving `.tx` files, the default glyph encoding mode is `t8::TxGlyphEncoding::TryUnicodePreferredAndFallbackElseAsciiOnly`. This keeps ASCII-only textures compact, but automatically stores preferred Unicode code points and ASCII fallbacks when any texture glyph needs the richer representation.

### Argument Options

`enum class t8::TextureFileFormat { Auto, Tx, Ansi }` (in `TextureFile.h`) determines which format the texture will be loaded from or saved to.
`t8::TextureFileFormat::Auto` (default) deduces the format from the file extension. You can also request a specific format explicitly.

`enum class t8::AnsiLoadGlyphEncoding { Auto, UTF8, CP437 }` (in `TextureFileAnsi.h`) determines which encoding will be used when loading an ansi file.
`t8::AnsiLoadGlyphEncoding::Auto` (default) deduces the encoding format from either the file extension or the BOM if present. If `*.utf8ans` is used or if there is an UTF-8 BOM at the file header of the ANSI file then UTF-8 encoding will be used, otherwise CP437 encoding will be used.

```cpp
enum class t8::AnsiSaveGlyphEncoding
{
  AutoPreserveGlyphs,  // CP437 if lossless, else UTF8 + BOM. If *.utf8ans, always UTF8.
  AutoPreferCP437,     // CP437, using fallback for non-CP437 glyphs. If *.utf8ans, always UTF8.
  UTF8,                // Explicit UTF8. No BOM for *.utf8ans, otherwise BOM.
  CP437,               // Explicit CP437. Errors if preferred CPs cannot be converted to CP437.
};
```
Similar to `t8::AnsiLoadGlyphEncoding` but with `Auto` split into two options: `AutoPreserveGlyphs` and `AutoPreferCP437`. `AutoPreserveGlyphs` make sure that the preferred glyph code point is preserved, which means that if it's outside of CP437 then the whole file will be saved in UTF-8 instead. `AutoPreferCP437` means the fallback ASCII will be used if unable to preserve the preferred code point as a CP437 code point.

```cpp
enum class TxGlyphEncoding
{
  AsciiOnly,                                   // 1 byte per cell.
  EnforceUnicodePreferredAndFallback,          // Store preferred + fallback always.
  TryUnicodePreferredAndFallbackElseAsciiOnly, // Store preferred + fallback if any preferred is non-printable ASCII.
};
```
This is used for determining how to save each glyph when saving to the TX format.
* `t8::TxGlyphEncoding::AsciiOnly` means that only the fallback ASCII characters will be stored.
* `t8::TxGlyphEncoding::EnforceUnicodePreferredAndFallback` means that both preferred and fallback will be stored.
* `t8::TxGlyphEncoding::TryUnicodePreferredAndFallbackElseAsciiOnly` (default) means that if there is any preferred code point that is not printable ASCII, then the file is saved in the same way as `t8::TxGlyphEncoding::EnforceUnicodePreferredAndFallback`. If all preferred glyphs are printable ASCII, then `t8::TxGlyphEncoding::AsciiOnly` is used instead.

The native format is .tx. ANSI-style formats are also supported, but may require sidecar files to preserve fallback glyphs and material IDs. See [TX_FORMAT.md](TX_FORMAT.md) and [ANSI.md](ANSI.md) for details.

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
