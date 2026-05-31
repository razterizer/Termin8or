# Termin8or

![GitHub License](https://img.shields.io/github/license/razterizer/Termin8or?color=blue)
![Static Badge](https://img.shields.io/badge/linkage-header_only-yellow)
![Static Badge](https://img.shields.io/badge/C%2B%2B-20-yellow)

[![build and test ubuntu](https://github.com/razterizer/Termin8or/actions/workflows/build-and-test-ubuntu.yml/badge.svg)](https://github.com/razterizer/Termin8or/actions/workflows/build-and-test-ubuntu.yml)
[![build macos](https://github.com/razterizer/Termin8or/actions/workflows/build-macos.yml/badge.svg)](https://github.com/razterizer/Termin8or/actions/workflows/build-macos.yml)
[![build windows](https://github.com/razterizer/Termin8or/actions/workflows/build-windows.yml/badge.svg)](https://github.com/razterizer/Termin8or/actions/workflows/build-windows.yml)

![Top Languages](https://img.shields.io/github/languages/top/razterizer/Termin8or)
![GitHub repo size](https://img.shields.io/github/repo-size/razterizer/Termin8or)
![C++ LOC](https://raw.githubusercontent.com/razterizer/Termin8or/badges/loc-badge.svg)
![Commit Activity](https://img.shields.io/github/commit-activity/t/razterizer/Termin8or)
![Last Commit](https://img.shields.io/github/last-commit/razterizer/Termin8or?color=blue)
![Contributors](https://img.shields.io/github/contributors/razterizer/Termin8or?color=blue)

![GitHub Downloads (all assets, all releases)](https://img.shields.io/github/downloads/razterizer/Termin8or/total)

`Termin8or` is a cross-platform, header-only C++20 library for terminal
graphics, text-mode UI, sprites, textures and small real-time terminal
applications.

It is curses-like in spirit, but the API is built around a screen buffer,
colored glyphs, sprites, simple UI widgets, and optional game-engine style
application scaffolding. Modern Termin8or code can work with either ASCII-only
glyphs or Unicode glyphs with ASCII fallbacks, depending on platform and
terminal capabilities.

Supported platforms (including but not limited to) are:
* Linux.
* MacOS.
* Windows.

## Highlights

* Header-only C++20 library.
* Buffered screen rendering with full and partial redraw strategies.
* 256-color support plus special colors for default/transparent rendering.
* Unicode-aware `Glyph` model with preferred codepoint and ASCII fallback.
* `Texture` / `Textel` data model with glyphs, foreground colors, background
  colors and material IDs.
* Texture file I/O through `TextureFile`, including `.tx` and ANSI-art files.
* ANSI import/export with CP437 and UTF-8/BOM handling.
* Bitmap and vector sprites, including animation and material-aware collisions and dynamics.
* Lightweight UI widgets: dialogs, text fields, buttons, color pickers and
  glyph pickers.
* Optional `GameEngine` helper for frame loops, input, logging, pause/quit,
  animations and lifecycle handling.

## Showcase

[Pilot Episode](https://github.com/razterizer/Pilot_Episode)

![image](https://github.com/razterizer/Pilot_Episode/assets/32767250/9b596d5c-e128-4dba-8fb5-02e9b3d8f412)
![image](https://github.com/razterizer/Pilot_Episode/assets/32767250/abd78df7-7314-43a2-8ef6-0270dc979575)

[SurgSim Lite](https://github.com/razterizer/SurgSim_Lite)

<img width="568" alt="image" src="https://github.com/razterizer/SurgSim_Lite/assets/32767250/2d356a54-6117-4d6b-aeca-4a025e8ae372">
<img width="567" alt="image" src="https://github.com/razterizer/SurgSim_Lite/assets/32767250/47cf2c18-c9cf-46a5-9787-bfd3ad69540a">

[DungGine demo](https://github.com/razterizer/DungGine)

<img width="568" alt="image" src="https://github.com/user-attachments/assets/c3075838-7f15-4679-862e-ce47780c9b39">

[TextUR](https://github.com/razterizer/TextUR)

<img width="568" alt="image" src="https://github.com/user-attachments/assets/bfb04801-8969-418f-b5bd-0c24d37eb0b3">

[Asciiroids](https://github.com/razterizer/Asciiroids)

<img width="703" height="594" alt="image" src="https://github.com/user-attachments/assets/369e49c5-8242-4997-8fa4-d46e4f40138d" />

## Sprite System

### Bitmap Sprites

![ubuntu_elli1an7bg_360](https://github.com/user-attachments/assets/19d3a6d6-b6f1-46a6-a80f-bcc15e202404)

Attaching the spaceship to a rigid body object:

![bitmap_sprite_dynamics](https://github.com/user-attachments/assets/74bfb3a8-51fe-4ffd-a06f-8792a476febb)

### Vector Sprites

![ubuntu_vhurwusqie_360](https://github.com/user-attachments/assets/35a03da6-d37d-479c-b84e-10891ac50825)

Now also supports filled vector sprites with character and style of your choice!

<img width="320" alt="image" src="https://github.com/user-attachments/assets/09de3a27-2951-4b99-8b17-f072ec1acdea" />

### Dynamics / Collision System

<!-- ![Kapture 2024-10-31 at 14 08 03](https://github.com/user-attachments/assets/2cd50164-db0f-4672-93e4-e0c508108079) -->
![vector_sprite_collisions](https://github.com/user-attachments/assets/0d562d19-059d-4af8-bb12-38578b5d1471)

Debug-drawing the broad-phase (AABB BVH) and the narrow-phase (character/character overlap tests) collision detection:

![vector_sprite_collisions_dbg](https://github.com/user-attachments/assets/20e3ba2c-d2d4-463c-939e-2f22b2a74c7d)

---
## Namespaces

There are now two namespaces: `t8` and `t8x`. `t8` contain the core features and `t8x` contain extra features that likely be lifted out from this repo to a separate repo in the future in order to keep `Termin8or` (core) lightweight enough.

## Header Files

### Geometry

* `geom/RC.h` (`t8`) : Row/column coordinates struct used for screen position, texture coordinates and such.
* `geom/Rectangle.h` (`t8`) : Rectangle struct for screen regions and bounding boxes.
* `geom/AABB.h` (`t8x`) : Templated AABB helper (used by broad-phase collision detection).

### Screen, Text And Glyphs

* `screen/Color.h` (`t8`) : 256-color ANSI palette plus special colors `Default` (-1), `Transparent` (-2) and `Transparent2` (-3). The colors are divided into: 3 special colors, 16 4-bit colors, 6^3 rgb color cube (rgb6) and 24 gray scale shades (gray24) (colors indices in range [-3, 255]). The two transparency colours / modes allows you to overlay text using the same colors that are already present in a given location in the screen buffer (see `ScreenHandler.h`).
* `screen/Styles.h` (`t8`) : `Style` and related helper structs for grouping foreground and background colors.
* `screen/Glyph.h` (`t8`) : Unicode-aware glyph value. A glyph can have a preferred Unicode codepoint and an ASCII fallback.
* `screen/GlyphString.h` (`t8`) : String-like container of `Glyph` values. Can be used as template argument for some classes and structs such as `TextBox`, `Dialog`, `MessageHandler` and `ParticleGradientGroup` (by default, these use `std::string`).
* `screen/StyledString.h` (`t8`) : Small helper struct for strings with color/style metadata. This struct works like an intermediate clustered string based on `std::string` that may or may not contain `UTF-8` encoded glyphs, so it's not as pure as `GlyphString` or just `std::string` in that regard.
* `screen/TermHelper.h` (`t8::term`) : Terminal capability helpers for locale setup, single-column glyph checks, Unicode encoding and ASCII fallback resolution. It has a `force_ascii_fallback` state that allows you to force only ASCII output in runtime (set on top level by `ScreenHandler` via the `AsciiFallbackPolicy` enum class).
* `screen/Ansi.h` (`t8::ansi`) : ANSI SGR color generation/parsing and small CSI parsers used by ANSI texture loading.
* `screen/Text.h` (`t8`) : Low-level text output implementation for ANSI terminals and Windows console (WIN-API) paths.
* `screen/ScreenHandler.h` (`t8`) : Handles text output and provides a screen buffer, transparency handling and frame output. It supports `char` and `char32_t` code paths. It offers policies such as `DrawPolicy` and `AsciiFallbackPolicy`. `AsciiFallbackPolicy` allows you to override the `CharT = char32_t` template argument via runtime code path by showing only ASCII characters (`Glyph` preferred first, then fallback).
* `screen/ScreenScaling.h` (`t8x`) : Class that allows you to scale up/down the screen buffer. Beware that any actual text or ASCII banner will not be readable when scaled up or down!
* `screen/ScreenCommandsBasic.h` (`t8`) : Low-level terminal commands such as clear, cursor movement and cursor visibility.
* `screen/ScreenCommands.h` (`t8`) : Higher-level terminal setup/teardown helpers such as `begin_screen()` and `end_screen()`.
* `screen/ScreenUtils.h` (`t8`, `t8x`) : Drawing helpers for frames, pause screens, confirmations, hiscore input and game-over banners.

### Drawing And Textures

* `drawing/Drawing.h` (`t8x`) : Drawing functions such as `plot_line()`, `draw_box()`, `draw_box_textured()`, `draw_box_outline()` and `filled_circle_positions()`.
* `drawing/Texture.h` (`t8`) : `Texture` and `Textel` data model. Stores glyphs, foreground colors, background colors and raw material IDs.
* `drawing/TextureFile.h` (`t8`) : Public texture file I/O dispatcher. Deduces or accepts a `TextureFileFormat` and calls the format-specific loader/saver.
* `drawing/texture_file/TextureFileTx.h` (`t8`) : Native `.tx` texture file load/save support.
* `drawing/texture_file/TextureFileAnsi.h` (`t8`) : ANSI-art load/save support for `.ans`, `.ansi`, `.txt`, `.diz`, `.asc`, `.nfo` and `.utf8ans`.
* `drawing/texture_file/TextureFileCommon.h` (`t8`) : Shared texture file helpers.
* `drawing/Gradient.h` (`t8x`) : Allows you to access a vector of values/objects using a normalized (0 to 1) t parameter. Useful for particles and color gradients.
* `drawing/LineData.h` (`t8x`) : Helper for streaming line-oriented pixel data to `ScreenHandler`. Depends on `Pixel`. See below.
* `drawing/Pixel.h` (`t8x`) : Pixel/textel-like helper used by older drawing code. Dependency to `LineData`.

### Input, UI And Application Helpers

* `input/Keyboard.h` (`t8`) : Non-blocking keyboard polling through `StreamKeyboard`. Function `readKey()` scans keypresses in an un-blocked manner and returns a struct `KeyPressDataPair` containing two objects of type `KeyPressData`; `transient` and `held`. The former being the raw key presses and the latter being the raw key presses buffered in a buffer that has a size proportional to the FPS of the application. These two modes have their pros and cons: Transient mode is more accurate but cannot capture held key presses, held mode on the other hand is great at capturing held keys but due to buffering, it suffers from minor inaccuracies.
* `input/KeyboardEnums.h` (`t8`) : `SpecialKey` enum and related keyboard symbols.
* `ui/MessageHandler.h` (`t8x`) : The `MessageHandler` class allows you to queue up messages of different severity levels and durations. Messages are displayed via a `TextBox`. It works as a timed message queue.
* `ui/UI.h` (`t8x`) : Compatibility include for the widget headers.
* `ui/widget/Widget.h` (`t8x`) : Base widget class.
* `ui/widget/Button.h` (`t8x`) : Button widget.
* `ui/widget/ButtonGroup.h` (`t8x`) : Group of selectable buttons.
* `ui/widget/Label.h` (`t8x`) : Label widget.
* `ui/widget/TextField.h` (`t8x`) : Text field widget with numeric, hex, printable ASCII and other input modes.
* `ui/widget/GlyphPicker.h` (`t8x`) : Glyph picker with preferred Unicode glyph, ASCII fallback and recent-glyph handling.
* `ui/widget/ColorPicker.h` (`t8x`) : Color picker widget.
* `ui/widget/TextBox.h` (`t8x`) : Text box/panel rendering helper.
* `ui/widget/Dialog.h` (`t8x`) : Dialog container for labels, text fields, buttons, color pickers and glyph pickers.
* `ui/widget/TextBoxDebug.h` (`t8x`) : Debug-oriented text box for live parameters.
* `sys/GameEngine.h` (`t8x`) : A customizable but easy-to-use frame-loop game engine for terminal apps and games (or engine for any real-time terminal-based program). Handles input, logging, pause/quit, animation counters and lifecycle hooks.
* `sys/Logging.h` (`t8x`) : Allows you to record the current random seed, frame numbers and respective keypresses in your program and then replay it. This makes finding runtime bugs a breeze.

### Sprites And Physics

* `sprite/SpriteHandler.h` (`t8x`) : Bitmap and vector sprites (`BitmapSprite` and `VectorSprite`), sprite animation, vector fills and sprite-frame I/O. A sprite can be controlled programmatically or be attached to a `RigidBody` object. Both sprite classes support sprite animations. A vector sprite can be filled for enclosed areas of the sprite. A scan-line algorithm is used for this.
* `physics/ParticleSystem.h` (`t8x`) : ASCII/terminal-style particle system for effects such as liquid, fire and smoke. See ([SurgSim_Lite](https://github.com/razterizer/SurgSim_Lite), [Pilot_Episode](https://github.com/razterizer/Pilot_Episode) and [DungGine](https://github.com/razterizer/DungGine) for examples).
* `physics/dynamics/RigidBody.h` (`t8x`) : Rigid body representation used by the dynamics system. You can attach a sprite to it if you want the sprite to be physically dynamic. The sprite also determines the "pixels" that make out the collision surface.
* `physics/dynamics/DynamicsSystem.h` (`t8x`) : Rigid body update/integration owner.
* `physics/dynamics/CollisionHandler.h` (`t8x`) : Broad-phase AABB BVH and narrow-phase material/glyph overlap collision detection and response. Collision response uses an impulse equation as a function of the velocities of the two bodies and their collision normals.

### Other

* `title/ASCII_Fonts.h` (`t8x`) : API for rendering text using FIGlet fonts and allows for styling the text with different colors. Supported FIGlet fonts are:
  * Larry3D.
  * SMSlant.
  * Avatar.
* `str/StringConversion.h` (`t8`) : Helpers for converting Termin8or objects to `std::string`.

## Make New Release

To make a new release, you first have to update the `RELEASE_NOTES.md` document. E.g.:

<img width="759" height="406" alt="image" src="https://github.com/user-attachments/assets/f5fbd6e3-cf10-473b-8e7d-908189c82d20" />


Then to make a new release you should use the `tag_release.sh` script:
```sh
./tag_release -f RELEASE_NOTES.md
```

or if you want to re-upload an existing release, e.g.:
```sh
./tag_release 1.1.5.7. -f RELEASE_NOTES.md
```

you can also use these commands:
```sh
./tag_release bump patch "Release notes."
```
```sh
./tag_release bump minor "Release notes."
```
```sh
./tag_release bump major "Release notes."
```
```sh
./tag_release 1.1.5.7 "Release notes."
```

If you want to remove a release for some reason, you can do so by running the following git commands:
```sh
# Delete local tag
git tag -d release-1.0.0.0

# Delete remote tag
git push --delete origin release-1.0.0.0
```

## Build & Run Instructions

`Termin8or` only depends on [`Core`](https://github.com/razterizer/Core).

There are two options when dealing with this repo dependency:

### Repo Dependencies Option 1

This method will ensure that you are running the latest stable version of the dependency `Core`.

The script `fetch-dependencies.py` used for this was created by [Thibaut Buchert](https://github.com/thibautbuchert).

You cd to the folder below the repo root folder of your `Termin8or` checkout and then run:

```
./Termin8or/fetch-dependencies.py Termin8or/dependencies
```
The folder structure should now look something like this, where Core and Termin8or are sibling folders:
```
<my_source_code_dir>/lib/Core/                   ; Core repo workspace/checkout goes here.
<my_source_code_dir>/lib/Termin8or/              ; Termin8or repo workspace/checkout goes here.
```

### Repo Dependencies Option 2

This method is more suitable for development as we're not necessarily working with a "locked" dependency but rather the latest version of the dependency.

You need the following header-only library:
* https://github.com/razterizer/Core

Make sure the folder structure looks like this:
```
<my_source_code_dir>/lib/Core/                   ; Core repo workspace/checkout goes here.
<my_source_code_dir>/lib/Termin8or/              ; Termin8or repo workspace/checkout goes here.
```

## How ScreenHandler Works

The `ScreenHandler` uses `Text.h` for translation of text and color information to strings of escape sequences.
You mainly update the screen buffer by calling `ScreenHandler::write_buffer()` for every string, glyph string, texture, sprite or other chunk with a fixed style.
So a game or application typically calls `write_buffer()` many times in a single frame. It is important to note that you call it in reverse-painter's-algorithm order, which means you write to the buffer in the order of foreground to background,
e.g. you write your playable character first and then end with scenery such as mountains etc. The reason for this is that every call to `write_buffer()` checks cell by cell for already occupied glyphs/textels before allowing new cells to be written into the buffer.

Later in your program - i.e. before the end of the current frame and after all `write_buffer()` calls have been made in that frame - then call `print_screen_buffer(Color clear_bg_color, DrawPolicy redraw_policy)`.

It will make a full redraw or a partial redraw depending on which policy you've selected. Partial redraw means dirty region tracking, achieved by diffing the current screen buffers with the previous frame values. This is often faster than a full redraw. When a partial redraw is being made, chunks of contiguous cells/textels/tiles will be drawn using `gotorc()` and at the end of the frame the frame will be flushed.
 
 I made a quick benchmark on my computer (MacBook Air M1) by running `Pilot_Episode` with `GameEngineParams::enable_benchmark = true` and using the default draw policy (`t8::DrawPolicy::MEASURE_SELECT`). This was the results:
 ```
Goal FPS = 10000
Average FPS = 1810.25
# Full Redraws = 18
# Partial Redraws = 35124
```
So I would say, it's fairly fast.

## Texture File Formats

Termin8or currently supports:

* `.tx` : Native Termin8or texture format.
* `.ans`, `.ansi`, `.txt`, `.diz`, `.asc`, `.nfo` : ANSI/CP437-style text art (or UTF-8 encoded text art if UTF-8 BOM is available).
* `.utf8ans` : ANSI-style text art where glyph bytes are interpreted as UTF-8.

ANSI import/export supports CP437 and UTF-8/BOM based workflows. Fallback and
material data can be stored in sidecar files next to the ANSI file:

* `<file>.fb` : ASCII fallback glyphs for Unicode glyphs.
* `<file>.mat` : Texture material IDs.

## Additional Information

* [Glyphs](docs/GLYPHS.md)
* [Textures](docs/TEXTURES.md)
* [TX File Format Specification](docs/TX_FORMAT.md)
* [ANSI](docs/ANSI.md)
* [Migration Notes for the Unicode-Supported API](docs/MIGRATION_UTF8.md)
