# Migration Notes For The Unicode-Supported API

This document summarizes the main changes needed when moving older Termin8or code to the Unicode-supported API.

The big conceptual change is that Termin8or no longer treats every drawable character as a plain `char`. The central glyph model is now `t8::Glyph`, which stores:

* a preferred Unicode code point (`char32_t preferred`)
* a printable ASCII fallback (`char fallback`)

This allows Unicode-capable terminals to render richer glyphs while ASCII-only paths remain deterministic.

## Quick Checklist

When migrating an older project, check these areas first:

* Replace raw character assumptions with `t8::Glyph` where the code represents drawable cells.
* Use `t8::GlyphString` explicitly for glyph-aware strings.
* Replace implicit `GlyphString(std::string)` usage with `GlyphString::from_ascii(...)`, `GlyphString::from_utf8(...)`, or `GlyphString::from_number(...)`.
* Treat `GlyphString` as single-line text only.
* Use decoded material APIs unless you intentionally manipulate raw material bytes.
* Update texture file I/O calls to use `t8::TextureFile`.
* Decide whether `.tx`, ANSI/CP437, or UTF-8 ANSI is the right texture format for the workflow.
* Review any code that assumes old material value semantics.
* Test with both `ScreenHandler<..., char>` and `ScreenHandler<..., char32_t>` if your project supports both.

## Glyph

Older code often used `char` directly as the thing drawn into a cell. Newer code should use `t8::Glyph` when the value represents a drawable glyph that may need Unicode/fallback behavior.

Examples:

```cpp
t8::Glyph ascii { '#' };
t8::Glyph block { U'█', '#' };
t8::Glyph heart { U'♥', 'v' };
```

Valid glyph states are stricter than raw character storage:

* Printable ASCII glyphs canonicalize to `{ preferred = ASCII, fallback = same ASCII }`.
* Non-ASCII preferred glyphs require a printable ASCII fallback.
* ASCII control characters are not valid preferred glyphs.
* Fallback must be printable ASCII.

See [GLYPHS.md](GLYPHS.md) for the full model.

## GlyphString

`t8::GlyphString` is now deliberately strict about string construction.

These constructors are deleted:

```cpp
GlyphString(std::string_view) = delete;
GlyphString(const std::string&) = delete;
```

This avoids silently treating UTF-8 bytes as separate glyphs.

Use explicit construction helpers instead:

```cpp
auto ascii = t8::GlyphString::from_ascii("HP: ");
auto utf8  = t8::GlyphString::from_utf8("Hjärta: ♥", 'v');
auto num   = t8::GlyphString::from_number(hp);
```

For ASCII literals, the `_gs` literal is still convenient:

```cpp
using namespace t8::literals;

auto label = "HP: "_gs + t8::Glyph { U'♥', 'v' } + " "_gs
           + t8::GlyphString::from_number(hp);
```

`GlyphString` represents one line of text. Do not put `\n` or `\r` inside it. For multiline glyph-aware text, use `std::vector<t8::GlyphString>` where the API supports line vectors.

## TextBox, StringBox And MessageHandler

Text-related UI code may need small adjustments when using `GlyphString`.

For `std::string`, `StringBox` can still split strings on newline characters. For `GlyphString`, newline is not a glyph; use a vector of lines instead.

For message-style UI, prefer explicit multiline APIs when the message has multiple lines:

```cpp
std::vector<t8::GlyphString> lines {
  t8::GlyphString::from_ascii("Saved file:"),
  t8::GlyphString::from_ascii(path)
};

message_handler.add_message_multi_line(time, lines, level);
```

Single-line messages can still be passed as a single `StrT`.

## ScreenHandler Character Paths

`ScreenHandler<NR, NC, CharT>` supports both:

* `CharT = char`
* `CharT = char32_t`

With `CharT = char`, Termin8or renders ASCII-only output. Printable ASCII preferred glyphs are used directly; otherwise the printable ASCII fallback is used.

With `CharT = char32_t`, Termin8or can try to render preferred Unicode glyphs, subject to terminal/font support and single-width constraints.

ASCII fallback behavior can also be controlled at runtime:

```cpp
screen_handler.set_ascii_fallback_policy(t8::AsciiFallbackPolicy::FORCE_ASCII);
```

When using `GameEngine`, set this through:

```cpp
t8x::GameEngineParams params;
params.ascii_fallback_policy = t8::AsciiFallbackPolicy::FORCE_ASCII;
```

## Materials

Texture materials now have a clearer split between raw storage and public decoded values.

Raw storage:

* Stored as `uint8_t`.
* `t8::texture::raw_mat_none == 255`.

Public decoded values:

* Stored as `int`.
* `t8::texture::mat_none == -1`.
* Valid material IDs are `0..254`.

Use these helpers when crossing the boundary:

```cpp
auto raw_mat = t8::texture::encode_raw_material(mat);
auto mat = t8::texture::decode_raw_material(raw_mat);
```

Prefer public decoded APIs such as `Texture::set_textel_material(...)` and `Texture::get_textel_material(...)` unless you intentionally operate on raw texture storage.

If your code writes directly to `Texture::materials_raw`, make sure values are encoded first.

## Texture File I/O

Texture file I/O is now handled through `t8::TextureFile` and format-specific helpers under `include/Termin8or/drawing/texture_file/`.

Typical usage:

```cpp
t8::Texture tex;
t8::TextureFile::load(tex, "sprite.tx");
t8::TextureFile::save(tex, "sprite.tx");
```

`TextureFileFormat::Auto` deduces the format from the extension:

* `.tx` -> native Termin8or texture format.
* `.ans`, `.ansi`, `.txt`, `.diz`, `.asc`, `.nfo` -> ANSI-style text art.
* `.utf8ans` -> UTF-8 ANSI-style text art.

The default `.tx` save mode is:

```cpp
t8::TxGlyphEncoding::TryUnicodePreferredAndFallbackElseAsciiOnly
```

This keeps ASCII-only textures compact, but preserves preferred Unicode glyphs and fallbacks when needed.

See [TEXTURES.md](TEXTURES.md), [TX_FORMAT.md](TX_FORMAT.md), and [ANSI.md](ANSI.md).

## ANSI Import / Export

ANSI texture support now handles both CP437 and UTF-8 workflows.

Loading uses:

```cpp
t8::AnsiLoadGlyphEncoding::Auto
```

Auto loading chooses UTF-8 when a UTF-8 BOM is present or when the extension is `.utf8ans`; otherwise it defaults to CP437.

Saving uses:

```cpp
t8::AnsiSaveGlyphEncoding::AutoPreserveGlyphs
```

This saves CP437 when all preferred glyphs can be represented as CP437; otherwise it saves UTF-8 with a BOM for `.ans`/`.ansi`/`.txt`/`.diz`/`.asc`/`.nfo`.

ANSI files may use sidecars:

* `<file>.fb` for ASCII fallbacks.
* `<file>.mat` for material IDs.

See [ANSI.md](ANSI.md) for details.

## Common Compile Errors

### Newline Assertions In GlyphString

If `GlyphString::from_ascii(...)` asserts on `\n`, split the text into lines first:

```cpp
std::vector<t8::GlyphString> lines {
  t8::GlyphString::from_ascii("Line 1"),
  t8::GlyphString::from_ascii("Line 2")
};
```

### Material Values Wrap Or Look Wrong

If material IDs above `254` or `-1` behave unexpectedly, check whether the code is writing decoded values directly into `materials_raw`.

Use:

```cpp
t8::texture::encode_raw_material(mat)
```

before storing in raw material vectors.

## Suggested Migration Order

1. Update Core and Termin8or together so string helpers, UTF-8 helpers and CP437 helpers match.
2. Fix compile errors from deleted `GlyphString` string constructors.
3. Replace raw character cell data with `Glyph` where Unicode/fallback behavior is needed.
4. Audit texture material code for raw/decoded material confusion.
5. Update texture load/save calls and choose the desired file format.
6. Run examples/tests in ASCII mode and Unicode mode.
7. Manually test terminal output on the platforms you care about.

## Remaining Platform Caveat

Unicode support depends on terminal and font behavior. Even when a code point is valid and single-width, different terminals may render it differently, or the selected font may draw it with different proportions.

Always keep meaningful ASCII fallbacks for non-ASCII glyphs.
