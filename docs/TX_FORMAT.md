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

The color encodings are done in different categories:

### 4-bit Colors + Extra Color Modes

The extra color modes are:
| Encoding | Color | Notes |
| --- | --- | --- |
| `T` | `Color16::Transparent2 = -3` | Transparency mode 2 |
| `t` | `Color16::Transparent = -2` | Transparency mode 1 |
| `D` | `Color16::Default = -1` | Default color |

The 4-bit colors are:
| Encoding | Color |
| --- | --- |
| `0` | `Color16::Black = 0` |
| `1` | `Color16::DarkRed = 1` |
| `2` | `Color16::DarkGreen = 2` |
| `3` | `Color16::DarkYellow = 3` |
| `4` | `Color16::DarkBlue = 4` |
| `5` | `Color16::DarkMagenta = 5` |
| `6` | `Color16::DarkCyan = 6` |
| `7` | `Color16::LightGray = 7` |
| `8` | `Color16::DarkGray = 8` |
| `9` | `Color16::Red = 9` |
| `A` | `Color16::Green = 10` |
| `B` | `Color16::Yellow = 11` |
| `C` | `Color16::Blue = 12` |
| `D` | `Color16::Magenta = 13` |
| `E` | `Color16::Cyan = 14` |
| `F` | `Color16::White = 15` |

These colors are supported by all TX versions.

### 8-bit Color Extensions

These colors are only supported by version 2.0 of the TX format.

The extended modes are as follows:

### RGB6

This is the ANSI RGB cube that is 6x6x6 colors, i.e. 6 Reds, 6 Greens and 6 Blues.

They are encoded like this: `"["<r><g><b>"]"`, so `[123]` means Red = 1, Green = 2, Blue = 3.

`<r>`, `<g>` and `<b>` are in the range `[0, 5]`.

### Gray24

This is a gray scale gradient from the ANSI standard that goes from almost black to almost white.

They are encoded like this: `"{"<gray>"}"`, so `{0}` is very dark gray and `{23}` is very bright gray.

`<gray>` is in the range `[0, 23]`.

## Material Block
