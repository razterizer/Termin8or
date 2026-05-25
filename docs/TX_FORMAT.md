# TX File Format

The TX format has the following layout:

```
<optional_version_line>
<size_line>
<empty_line>
<glyph_block>
<empty_line>
<fg_color_block>
<empty_line>
<bg_color_block>
<empty_line>
<material_block>
```

## Version Line

```
<optional_version_line> := ["VER " <major> "." <minor>]
```

Version line is optional when loading; absence means `VER 1.0`.

Current saver always writes `VER <major>.<minor>`.

Here are the characteristics of each version:
* `VER 1.0`: Original format, ASCII glyphs, original 16-color encoding with additional transparency modes and default mode.
* `VER 2.0`: 8-bit color support.
* `VER 2.1`: `-` material sentinel support (`-` meaning `mat_none`, typically for AdHoc textels in [`TextUR`](https://github.com/razterizer/TextUR)).
* `VER 3.0`: Bracketed `Glyph` serialization with preferred Unicode + fallback.

## Size Line

```
<size_line> := <num_rows> <num_cols>
```
For example:
```
3 11
```

Where each block has `<num_rows>` non-empty lines.
Each block row encodes exactly `<num_cols>` cells, but cell width in characters may vary for newer formats.

## Glyph Block

Glyph block depends on version:
* ver <= 2.1: one legacy ASCII glyph per cell.
* ver >= 3.0: glyphs serialized with `Glyph::str(false)`, e.g. `[A]`, `[2588,#]`, `[]` etc.

## FG and BG Color Blocks

## Material Block
