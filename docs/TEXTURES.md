# Textures

## Overview

What a `Texture` is: a 2D grid of textels/cells.

Mention that textures can be used for:
- drawing reusable terminal art
- sprite frames
- UI elements
- level/map chunks
- TextUR-authored assets

## Texture Data Model

A `Texture` stores:
- `size`
- `area`
- `glyphs`
- `fg_colors`
- `bg_colors`
- `materials_raw`

Each cell is a `Textel`-like combination of:
- `Glyph`
- foreground `Color`
- background `Color`
- material ID

## Textel

Explain `Textel`:
- small cell value object
- glyph + fg + bg + material
- useful for editing one cell at a time
- raw material storage vs decoded material API

## Materials

Important section.

Explain:
- Material IDs are optional.
- Public/decoded material:
  - `-1` means no material.
  - `0..254` are material IDs.
- Raw material:
  - stored as `uint8_t`
  - `255` means no material.
  - `0..254` are material IDs.
- Use public decoded APIs unless you specifically need raw storage.

Mention helpers:
- `texture::mat_none`
- `texture::raw_mat_none`
- `encode_raw_material(int)`
- `decode_raw_material(uint8_t)`
- `has_material(int)`
- `has_raw_material(uint8_t)`

## Creating Textures

Examples:
- empty/default texture
- fixed-size texture
- fill with glyph/style
- set individual textel/glyph/color/material

Keep examples short.

## Loading and Saving

Explain current public API:
- `Texture::load(...)` / `Texture::save(...)` if those still exist
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
