# Glyphs

## What is a `Glyph`?

A `Glyph` is a simple struct that contains a `char32_t preferred` variable for preferred code point which could be any Unicode character (including ASCII of course) and a `char fallback` variable which should strictly only be a printable ASCII character in the range `[0x20, 0x7E]`.

The `Glyph` struct can be found in the `include/Termin8or/screen/Glyph.h` header and is inside the `t8` namespace (namespace for core Termin8or features).

`Glyph` objects are designed to work with `t8::ScreenHandler<CharT>` for both supported character paths:

- `CharT = char`: output is ASCII-only. Termin8or first uses `preferred` if it is printable ASCII; otherwise it uses the printable ASCII fallback.
- `CharT = char32_t`: Termin8or tries to render the preferred Unicode code point when it is valid and displayable as a single terminal cell, has coverage for the current font used and it is supported by the terminal/console; otherwise it uses the printable ASCII fallback.

This lets Termin8or store richer glyph information without abandoning ASCII compatibility. A texture, sprite or UI widget can prefer Unicode box drawing, block, symbol or CP437-derived glyphs while still having a deterministic ASCII representation for terminals or builds that cannot render them.

## Valid `Glyph` States

`Glyph` can be constructed as empty using the default constructor. Then `preferred = t8::Glyph::none32` and `fallback = t8::Glyph::none`. These are two specialized sentinels. `none32` can only be used with `preferred` and `none` can only be used with `fallback`.

A glyph object can only be in these valid states. All other states are invalid:
* `{ preferred = none32, fallback = none }`.
* `{ preferred = ASCII, fallback = none }`.
* `{ preferred = ASCII, fallback = ASCII }` (preferred must not equal fallback in this case).
* `{ preferred = Unicode, fallback = ASCII }`.

## Canonicalization

## Encoding and Rendering

## String Helpers

## Parsing and Serialization


