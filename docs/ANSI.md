# ANSI Texture Files

Termin8or can load and save ANSI-style text art through `t8::TextureFileAnsi` (centralized in `t8::TextureFile`).

The implementation is intentionally a practical ANSI-art importer/exporter, not a full terminal emulator. It supports the escape sequences and metadata needed by typical ANSI-art files and by Termin8or roundtrips.

## Supported Extensions

The following extensions are treated as ANSI-style texture files:

| Extension | Default Meaning |
| --- | --- |
| `.ans` | ANSI / CP437-style text art. |
| `.ansi` | ANSI / CP437-style text art. |
| `.txt` | ANSI / CP437-style text art. |
| `.diz` | ANSI / CP437-style text art. |
| `.asc` | ANSI / CP437-style text art. |
| `.nfo` | ANSI / CP437-style text art. |
| `.utf8ans` | ANSI-style text art where glyph bytes are interpreted as UTF-8. |

If a UTF-8 BOM is present at the beginning of the file, `Auto` loading treats the glyph data as UTF-8.

When `t8::TextureFileFormat::Auto` is used, these extensions route through `TextureFileAnsi` if any of the above extensions are used (case-insensitive).

## Default Colors

The ANSI loader/saver accepts `Color ansi_default_fg` and `Color ansi_default_bg` arguments. These define how ANSI default foreground/background resets are interpreted. Their defaults are `Color16::Default` and `Color16::Transparent2`.

## Loading

ANSI loading is handled by:

```cpp
t8::TextureFile::load(tex, "file.ans");
```

or explicitly:

```cpp
t8::TextureFile::load(tex,
                      "file.ans",
                      t8::TextureFileFormat::Ansi,
                      true,
                      t8::AnsiLoadGlyphEncoding::Auto);
```

### Glyph Encoding On Load

ANSI loading uses:

```cpp
enum class t8::AnsiLoadGlyphEncoding
{
  Auto,
  UTF8,
  CP437,
};
```

`Auto` chooses the glyph encoding like this:

| File / Input | Auto Result |
| --- | --- |
| UTF-8 BOM present | UTF-8 |
| `.utf8ans` without BOM | UTF-8 |
| `.ans`, `.ansi`, `.txt`, `.diz`, `.asc`, `.nfo` without BOM | CP437 |

If UTF-8 is requested for `.ans`, `.ansi`, `.txt`, `.diz`, `.asc` or `.nfo` without a BOM, the loader warns in verbose mode. If CP437 is requested for a UTF-8 BOM file or a `.utf8ans` file, the loader also warns in verbose mode.

### CP437

CP437 files are decoded byte-by-byte through the CP437 to Unicode mapping in the Core library. Printable ASCII bytes map to printable ASCII code points. CP437 graphical bytes map to Unicode code points such as box drawing, shade and block characters.

When a decoded Unicode glyph needs an ASCII fallback, Termin8or first checks the `.fb` sidecar file if present. If no fallback is supplied there, Termin8or tries its built-in fallback table.

### UTF-8

UTF-8 files are decoded as UTF-8 code points. ASCII control bytes below `0x20` are treated as spaces in the glyph stream unless they are part of already-handled line splitting or escape sequence parsing.

UTF-8 ANSI files can still use ANSI SGR color and cursor sequences.

## Supported Escape Sequences

Termin8or supports a focused subset of CSI escape sequences.

### SGR Colors

SGR sequences use `ESC[...m`.

Supported parameters include:

| SGR | Meaning |
| --- | --- |
| `0` | Reset foreground/background to ANSI defaults. |
| `1` | Bright/bold foreground mode. |
| `22` | Disable bright/bold foreground mode. |
| `30`-`37` | 4-bit foreground colors. |
| `40`-`47` | 4-bit background colors. |
| `90`-`97` | Bright foreground colors. |
| `100`-`107` | Bright background colors. |
| `38;5;<idx>` | 256-color foreground. |
| `48;5;<idx>` | 256-color background. |
| `39` | Reset foreground to default foreground. |
| `49` | Reset background to default background. |

The 256-color indices use Termin8or's `Color` index range `0..255`.

Bright mode affects foreground colors in the traditional ANSI-art way: `ESC[1;33m` is treated as bright yellow foreground, not only as a separate text style flag.

### Cursor Movement

Supported cursor movement sequences:

| Sequence | Meaning |
| --- | --- |
| `ESC[A` / `ESC[nA` | Move cursor up. |
| `ESC[B` / `ESC[nB` | Move cursor down. |
| `ESC[C` / `ESC[nC` | Move cursor forward. |
| `ESC[D` / `ESC[nD` | Move cursor backward. |
| `ESC[H` / `ESC[row;colH` | Move cursor to 1-based row/column. |
| `ESC[f` / `ESC[row;colf` | Same as `H`. |
| `ESC[s` | Save cursor position. |
| `ESC[u` | Restore cursor position. |

Cursor positions in ANSI sequences are 1-based. Internally, Termin8or stores rows and columns as 0-based coordinates.

### Erase

Supported erase sequences:

| Sequence | Meaning In Loader |
| --- | --- |
| `ESC[2J` | Clear parsed rows and reset cursor to row 0, column 0. |
| `ESC[2K` | Erase current parsed row from the cursor position onward. |

Other erase modes may parse as erase sequences but currently have no effect unless explicitly handled.

### Unsupported Sequences

If the loader encounters an unsupported escape sequence, loading fails and reports the sequence in verbose mode.

This is deliberate: the loader tries to avoid silently producing a misleading texture when the file relies on terminal behavior that Termin8or does not implement.

## Wrapping And Dimensions

ANSI art often assumes an 80-column terminal. Termin8or uses an 80-column wrap width by default when auto-wrapping is needed.

If a SAUCE record provides character-file dimensions, the SAUCE width is used as the wrap width instead.

Auto-wrapping is enabled when:

* SAUCE size information is found.
* The file uses save/restore cursor continuation patterns.
* The file appears to be one long logical row that should be wrapped.

The resulting texture size is derived from the parsed rows after escape sequences, cursor movement and wrapping have been applied.

## DOS EOF And SAUCE

The loader reads ANSI files as binary data.

Before parsing glyphs, it:

* Detects SAUCE character-file size information when present.
* Strips SAUCE records from the byte stream.
* Splits bytes into lines on `\n`.
* Removes trailing `\r` from CRLF lines.
* Stops at DOS EOF byte `0x1A` if present.

SAUCE support is used mainly for size/wrap behavior and for removing metadata from the visual payload. Termin8or does not currently expose full SAUCE metadata as public texture metadata.

## Sidecar Files

ANSI files do not natively store all Termin8or texture data. Termin8or therefore supports sidecar files next to the ANSI file.

For a file named:

```txt
art.ans
```

`TextureFileAnsi` will read/write:

```txt
<base-filename>.<file-ext>.fb
<base-filename>.<file-ext>.mat
```

e.g.

```txt
art.ans.fb
art.ans.mat
```

### Fallback Sidecar: `.fb`

The `.fb` sidecar stores one printable ASCII fallback character per parsed glyph cell.

It follows the same physical row/cell order as the ANSI glyph stream.

On load:

* If a fallback character exists in the `.fb` sidecar, it is used.
* If no sidecar fallback exists for a glyph, Termin8or tries the built-in ANSI fallback table.
* If neither exists for a non-ASCII glyph, loading fails.

On save, Termin8or writes `.fb` so Unicode glyphs can roundtrip with their explicit ASCII fallbacks.

### Material Sidecar: `.mat`

The `.mat` sidecar stores Termin8or material IDs using the same compact material encoding as the `.tx` format.

This sidecar also follows the same physical row/cell order as the ANSI glyph stream.

On load, missing material entries default to `t8::texture::raw_mat_none`.

On save, Termin8or writes `.mat` so material data can roundtrip even though ANSI files themselves do not carry Termin8or material IDs.

See [TX_FORMAT.md](TX_FORMAT.md) for the compact material encoding.

## Saving

ANSI saving is handled by:

```cpp
t8::TextureFile::save(tex, "file.ans");
```

or explicitly:

```cpp
t8::TextureFile::save(tex,
                      "file.ans",
                      t8::TextureFileFormat::Ansi,
                      true,
                      t8::TxGlyphEncoding::TryUnicodePreferredAndFallbackElseAsciiOnly,
                      t8::AnsiSaveGlyphEncoding::AutoPreserveGlyphs);
```

The `TxGlyphEncoding` argument is ignored by ANSI saving; it is part of the shared `TextureFile::save()` signature and applies to `.tx` saving.

### Glyph Encoding On Save

ANSI saving uses:

```cpp
enum class t8::AnsiSaveGlyphEncoding
{
  AutoPreserveGlyphs,
  AutoPreferCP437,
  UTF8,
  CP437,
};
```

| Mode | Behavior |
| --- | --- |
| `AutoPreserveGlyphs` | For `.utf8ans`, save UTF-8. For `.ans`, `.ansi`, `.txt`, `.diz`, `.asc` and `.nfo`, save CP437 if all preferred glyphs can map to CP437; otherwise save UTF-8 with BOM. |
| `AutoPreferCP437` | For `.utf8ans`, save UTF-8. Otherwise prefer CP437 and use printable ASCII fallback for glyphs that cannot map to CP437. |
| `UTF8` | Save UTF-8. For non-`.utf8ans` files, Termin8or adds a UTF-8 BOM. |
| `CP437` | Save CP437. If any preferred glyph cannot map to CP437, saving fails. |

When saving UTF-8 to `.utf8ans`, Termin8or does not add a BOM. When saving UTF-8 to `.ans`, `.ansi`, `.txt`, `.diz`, `.asc` or `.nfo`, Termin8or adds a UTF-8 BOM so `Auto` loading can detect it.

### Colors On Save

The saver emits ANSI SGR sequences when foreground or background color changes.

It uses:

* 4-bit foreground/background color sequences for color indices `0..15`.
* `38;5;<idx>` and `48;5;<idx>` for 256-color indices.
* `39` and `49` when a color matches the configured ANSI default foreground/background.

Each saved output line ends with `ESC[0m`.

## Limitations

Termin8or's ANSI loader is not a complete terminal emulator.

Current limitations include:

* Unsupported escape sequences fail loading.
* Only a subset of cursor movement, erase and SGR behavior is implemented.
* Full SAUCE metadata is not exposed through the public texture API.
* ANSI files do not directly store Termin8or fallback/material data without sidecars.
* Visual output can still differ between terminals because fonts and terminal emulators render CP437-derived Unicode glyphs differently.

For the native Termin8or texture format, use `.tx`. For the `.tx` format specification, see [TX_FORMAT.md](TX_FORMAT.md).
