# Glyphs

## What is a `Glyph`?

A `Glyph` is a simple struct that contains a `char32_t preferred` variable for preferred code point which could be any Unicode character (including ASCII of course) and a `char fallback` variable which should strictly only be a printable ASCII character in the range `[0x20, 0x7E]`.

The `Glyph` struct can be found in the `include/Termin8or/screen/Glyph.h` header and is inside the `t8` namespace (namespace for core Termin8or features).

`Glyph` objects are designed to work with `t8::ScreenHandler<NR, NC, CharT>` for both supported character paths:

- `CharT = char`: output is ASCII-only. Termin8or first uses `preferred` if it is printable ASCII; otherwise it uses the printable ASCII fallback.
- `CharT = char32_t`: Termin8or tries to render the preferred Unicode code point when it is valid and displayable as a single terminal cell, has coverage for the current font used and it is supported by the terminal/console; otherwise it uses the printable ASCII fallback.

This lets Termin8or store richer glyph information without abandoning ASCII compatibility. A texture, sprite or UI widget can prefer Unicode box drawing, block, symbol or CP437-derived glyphs while still having a deterministic ASCII representation for terminals or builds that cannot render them.

## Valid `Glyph` States

`Glyph` can be constructed as empty using the default constructor. Then `preferred = t8::Glyph::none32` and `fallback = t8::Glyph::none`. These are two specialized sentinels. `none32` can only be used with `preferred` and `none` can only be used with `fallback`.

A `Glyph` object is valid when it is in one of these states:

* `{ preferred = none32, fallback = none }` : empty glyph.
* `{ preferred = ASCII, fallback = same ASCII }` : printable ASCII glyph, canonical form.
* `{ preferred = Unicode > 0x7F, fallback = ASCII }` : Unicode glyph with printable ASCII fallback.

The temporary/draft state `{ preferred = none32, fallback = ASCII }` can occur while editing/parsing user input, but should be canonicalized with `try_canonicalize_from_fallback()` before serialization or use as a final glyph.

Examples:
* `Glyph(0x2588, '#')` -> `{ preferred = 0x2588, fallback = '#' }`.
* `Glyph('#')` -> `{ preferred = '#', fallback = '#' }`.
* `Glyph('#', 'I')` -> `{ preferred = '#', fallback = '#' }`. Auto-canonicalization: preferred -> fallback.
* `Glyph('#', 0x2588)` -> `"ERROR in Glyph(char32_t, char) : Fallback must be ASCII (<=0x7F)."`.
* `Glyph(0x2588)` -> `"ERROR in Glyph(char32_t, char) : Preferred cannot be non-ASCII while fallback is unset."`.
* `Glyph(none32, '#')` -> `"ERROR in Glyph(char32_t, char) : Fallback without preferred."`.

## Canonicalization

Glyphs are canonicalized automatically when two printable ASCII characters are supplied as constructor arguments. If they differ, then `fallback` will be overwritten by `preferred`, thus ensuring they both contain the same ASCII character.

Canonicalization can also be done manually in the reverse direction by calling `Glyph::try_canonicalize_from_fallback()`. This is useful for the draft state `{ preferred = none32, fallback = ASCII }`, which can occur during parsing or partial user input.

If a draft state is `{ preferred = none32, fallback = ASCII }`, calling `Glyph::try_canonicalize_from_fallback()` copies `fallback` into `preferred`.

This feature is useful for parsing code and partial user input and is specifically used in Termin8or for partial `GlyphPicker` user input.

## Encoding and Rendering

A `Glyph` is not written directly to the terminal. Before rendering, Termin8or resolves it into a single-width output glyph suitable for the current `ScreenHandler<NR, NC, CharT>` path.

For `CharT = char`, output is ASCII-only:

1. If `preferred` is printable ASCII, use `preferred`.
2. Otherwise, if `fallback` is printable ASCII, use `fallback`.
3. Otherwise, use a safe replacement, like `' '` or `'?'`.

With `CharT = char`, the `ScreenHandler` behaves just as it did before the Unicode support was added, which is useful for those cases where you want the more simple machinery.

For `CharT = char32_t`, Termin8or first tries to use `preferred` as Unicode. The preferred code point must be valid, supported by the current terminal/font path, and render as a single terminal cell. If it cannot be used safely, Termin8or falls back to the printable ASCII fallback or a safe replacement if not printable.

Termin8or deliberately avoids rendering glyphs that are not single-width. Terminal screen buffers are cell-based, so wide or combining glyphs can corrupt alignment and dirty-region rendering.

There is another, subtler problem: some glyphs are technically treated as single-cell by the terminal, but the font draws them with ink that spills outside the cell. Termin8or currently focuses on filtering out non-single-width glyphs, not on detecting single-cell glyphs with visually overflowing font rendering. The latter depends heavily on terminal, font and platform behavior and is much harder to categorize reliably.

ASCII fallback can also be forced at runtime. This is useful for terminals with limited Unicode support, deterministic ASCII screenshots, debugging, or applications that want to expose an ASCII-only mode. `ScreenHandler` exposes this through `t8::ScreenHandler<NR, NC, CharT>::set_ascii_fallback_policy(AsciiFallbackPolicy policy)`, where `AsciiFallbackPolicy` can be `SYSTEM_CONTROLLED` (default), `FORCE_ASCII`, or `FORCE_ASCII_ONLY_ON_WIN_CMD`. When using `GameEngine`, this can also be set through `GameEngineParams::ascii_fallback_policy`.

### Terminal / OS Font Coverage Checks

Here is an overview of what glyphs are checked for for different operating systems, terminals and fonts.

* Windows [`cmd.exe`]: `Consolas`, `Lucida Console`, `Cascadia Mono`.
* Windows [`Windows Terminal`]: No font glyph-coverage checks due to the loose relationship between selected font and terminal instance and also because when detecting the font used from the code, the retrieved font info is global and may not be relevant to the currently used instance.
* MacOS [`Terminal`]: Mainly `SF Mono`, however, it seems other fonts may work just as well as missing glyphs in one font are covered by some other font. I could be wrong though.
* Linux: No standardized way to check which font is being used or no known way to find glyph coverage via terminal application.

## String Helpers

Termin8or has one directly glyph-related string struct `GlyphString` and one indirectly glyph-related string struct `StyledString`.

### GlyphString

`GlyphString` is defined in header `include/Termin8or/screen/GlyphString.h` in the namespace `t8`.

This struct lets you construct strings from `Glyph` objects. It provides operators that make mixed ASCII/glyph composition ergonomic.

For example:

```cpp
using namespace t8::literals;

auto gstr = "HP: "_gs + Glyph { U'♥', 'v' } + " "_gs + std::to_string(hp);
```

`ScreenHandler<NR, NC, CharT>::write_buffer()` has overloads that accept `GlyphString`.

### StyledString

`StyledString` is defined in header `include/Termin8or/screen/StyledString.h` in the namespace `t8`.

`StyledString` wraps an `std::string` together with a `Style` (fg-color + bg-color) and width information (for tracking of UTF-8 encoded glyphs).

Note that this is not a glyph container; it is styled (possibly) encoded text.

Use `StyledStringVec` for more complex multi-styled strings.

`ScreenHandler<NR, NC, CharT>::write_buffer()` has overloads that accept `StyledStringVec`.

## Parsing and Serialization


