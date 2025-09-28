# Termin8or

![GitHub License](https://img.shields.io/github/license/razterizer/Termin8or?color=blue)
![Static Badge](https://img.shields.io/badge/linkage-header_only-yellow)
![Static Badge](https://img.shields.io/badge/C%2B%2B-20-yellow)

[![build and test ubuntu](https://github.com/razterizer/Termin8or/actions/workflows/build-and-test-ubuntu.yml/badge.svg)](https://github.com/razterizer/Termin8or/actions/workflows/build-and-test-ubuntu.yml)

![Top Languages](https://img.shields.io/github/languages/top/razterizer/Termin8or)
![GitHub repo size](https://img.shields.io/github/repo-size/razterizer/Termin8or)
![C++ LOC](https://raw.githubusercontent.com/razterizer/Termin8or/badges/loc-badge.svg)
![Commit Activity](https://img.shields.io/github/commit-activity/t/razterizer/Termin8or)
![Last Commit](https://img.shields.io/github/last-commit/razterizer/Termin8or?color=blue)
![Contributors](https://img.shields.io/github/contributors/razterizer/Termin8or?color=blue)

This is a cross-platform header-only curses-like lib.

Supported platforms (including but not limited to) are:
* Linux.
* MacOS.
* Windows.

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

* `geom/RC.h` (`t8`) : A struct representing the row and column position on the screen or in a texture or bounding box to mention a few.
* `geom/Rectangle.h` (`t8`) : A rectangle struct that can be used for bounding boxes etc.
* `geom/AABB` (`t8x`) : A templetized (int, float) AABB class that can return a `Rectangle` object if needed. Used in `CollisionHandler` for broad-phase detection.
* `screen/Color.h` (`t8`) : Contains colour definitions for the 16 colors that Termin8or (and the terminal) supports. There are also two transparency colours / modes which allows you to overlay text using the same colours that are already present in a given location in the screen buffer (see `ScreenHandler.h`).
* `screen/Styles.h` (`t8`) : `Style` and its derivatives are just fancy structs grouping a foreground colour with a background colour.
* `screen/ScreenCommandsBasic.h` (`t8`) : A collection of functions for controlling terminal screen output: 
  * Low-level functions: `clear_screen()`, `return_cursor()`, `restore_cursor()`, `hide_cursor()`, `show_cursor()`, `gotorc()`.
  * Medium-level functions: `get_terminal_window_size()`, `resize_terminal_window()`.
* `screen/ScreenCommands.h` (`t8`) : A collection of functions for controlling terminal screen output on a bit higher level (one of which involve `ScreenHandler`):
  * Medium-level functions: `save_terminal_colors()`, `restore_terminal_colors()`.
  * High-level functions: `begin_screen()`, `end_screen()`. These take care of color restoration, clearing screen, hiding the cursor etc, except for the resizing of the terminal window.
* `screen/ScreenUtils.h` (`t8`, `t8x`) : A collection of functions for rendering dialogs and such:
  * `draw_frame()` (`t8`) : Draws a simple frame around your frame buffer.
  * `draw_game_over()`, `draw_you_won()` (`t8x`) : Draws wavy banners in the FIGlet font Grafitti. Used by `GameEngine` if those features are enabled.
  * `draw_paused()` (`t8x`) : Draws an animated pause screen.
  * `draw_confirm()`, `draw_input_hiscore()`, `draw_hiscores()` (`t8x`) : These are the more UI-related functions of this header.
* `screen/Text.h` (`t8`) : `Text` handles text output and translates Color enum items to corresponding color values depending on platform compiled. Is also responsible for creating the appropriate ANDI escape sequences for the TTY and such.
* `screen/ScreenHandler.h` (`t8`) : Contains the screen buffers (char / fg-color / bg-color) and manages transparency etc. It outputs the contents to the terminal via a privatly owned `Text` object.
* `screen/ScreenScaling.h` (`t8x`) : Friend with `ScreenHandler.h` which allows you to scale the screen buffer. Beware that any acutal text or ASCII banner will not be readable when scaled up or down!
* `input/Keyboard.h` (`t8`) : A keyboard handling API that is easy to use. Use class `StreamKeyboard` to poll key presses. Function `readKey()` scans keypresses in an un-blocked manner and returns a struct `KeyPressDataPair` containing two objects of type `KeyPressData`; `transient` and `held`. The former being the raw key presses and the latter being the raw key presses buffered in a buffer that has a size proportional to the FPS of the application. These two modes have their pros and cons: Transient mode is more accurate but cannot capture held key presses, held mode on the other hand is great at capturing held keys but due to buffering, it suffers from minor inaccurracies.
* `input/KeyboardEnums.h` (`t8`) : just the `SpecialKey` enum class for now so you don't have to include all of `Keyboard.h` to be able to interpret keypresses.
* `drawing/Gradient.h` (`t8x`) : Allows you to access a vector of given objects using a normalized (0 to 1) t parameter. Useful for particle systems and things like that where it is used for color gradients.
* `drawing/Drawing.h` (`t8x`) : Features some drawing functions such as `plot_line()`, `draw_box()`, `draw_box_textured()`, `draw_box_outline()` and `filled_circle_positions()`.
* `drawing/LineData.h` (`t8x`) : a struct that helps to convert bigger chunks of strings / colors along a single line of text to individual "pixels" that can be "streamed" to the `ScreenHandler`. This will likely be deprecated in the future in favour of more recent tools such as sprites.
* `drawing/Pixel.h` (`t8x`) : a struct containing data such as character (or std::string of a single character), foreground color, background color (r,c) position (and original position), line index to `LineData` object and char index to within a `LineData` object and an enabled flag. This will likely be deprecated in the future in favour of more recent tools such as sprites.
* `drawing/Texture.h` (`t8`) : The `Texture` struct allows you to save and load text-based texture in text format to and from disk. There are four parts in a texture: characters, foreground colors, background colors and material IDs. Header also contains texture element `Textel` (humouristic portmantau of "text" and "texel" suggested by my dear colleague Göran Wallgren).
* `ui/MessageHandler.h` (`t8x`) : The `MessageHandler` class allows you to queue up messages of different severity levels and durations. Messages are displayed in a `TextBox` in the middle of the screen.
* `ui/UI.h` (`t8x`) : Contains structs such as `TextBox` and `TextBoxDebug` to make it easy to display info on the screen and such. `MessageHandler` uses `TextBox`. There is also a new class `Dialog` which can hold widgets such as `TextField`, `Button` (via an instance of `ButtonGroup`) and `ColorPicker`.
* `title/ASCII_Fonts.h` (`t8x`) : API for rendering text using FIGlet fonts and allows you to style your text with different colors. Supported FIGlet fonts are:
  * Larry3D.
  * SMSlant.
  * Avatar.
* `sys/GameEngine.h` (`t8x`) : A highly customizable buy easy to use game engine (or engine for any real-time terminal-based program).
* `sys/Logging.h` (`t8x`) : Allows you to record the current random seed, frame numbers and respective keypresses in your program and then replay it. This makes finding runtime bugs a breeze.
* `str/StringConversion.h` (`t8`) : Allows you to convert objects to `std::string` strings.
* `sprite/SpriteHandler.h` (`t8x`) : Manages bitmap sprites (`BitmapSprite`) and vector sprites (`VectorSprite`). A sprite can be controlled programmatically or be attached to a `RigidBody` object. Both sprite classes support sprite animations. A vector sprite can be filled for enclosed areas of the sprite. A scan-line algorithm is used for this.
* `physics/ParticleSystem.h` (`t8x`) : This ASCII-style particle system allows you to make cool real-time VFX such as liquids and fire-smoke. See ([`SurgSim_Lite`](https://github.com/razterizer/SurgSim_Lite),  [`Pilot_Episode`](https://github.com/razterizer/Pilot_Episode) and [`DungGine`](https://github.com/razterizer/DungGine) for examples).
* `physics/dynamics/RigidBody.h` (`t8x`) : A class that represents a rigid body. You attach a sprite to it if you want the sprite to be physically dynamic. The sprite also determines the "pixels" that make out the collison surface.
* `physics/dynamics/DynamicsSystem.h` (`t8x`) : A class that governs the dynamical motions of the rigid bodies that are created from it.
* `physics/dynamics/CollisionHandler.h` (`t8x`) : A class that governs collison detection and collison response between the rigid bodies created via the `DynamicsSystem` class. Broad-phase uses an AABB BVH and narrow-phase checks overlaps beteen sprite characters of a specified material index. Collision response uses an impulse equation as a function of the velocities of the two bodies and their collision normals.

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
You mainly update the screenbuffer by calling `ScreenHandler::write_buffer()` (non-static) for every string that has a fixed style (one fg color + one bg color).
So a game or application typically calls `write_buffer()` many times in a single frame. It is important to note that you call it in reverse-painter's-algorithm order, which means you write to the buffer in the order of foreground to background,
e.g. you write your playable character first and then end with scenery such as mountains etc. The reason for this is that every call to `write_buffer()` checks character by character (or tiles, or textels if you will) for already occupying characters before allowing any new characters/tiles/textels to be written into the buffer.

Later in your program - i.e. before the end of the current frame and after all `write_buffer()` calls have been made in that frame - then call `print_screen_buffer(Color clear_bg_color, DrawPolicy posix_redraw_policy)`.

 It will make a full redraw or a partial of the screen depending on which policy you've selected. Partial redraw means dirty region-tracking which is achieved by diffing the current screen buffers with the previous frame values. This is in very often faster than a full redraw. When a partial redraw is being made, chunks of contiguous cells/textels/tiles will be drawn using gotorc() and at the end of the frame the frame will be flushed.
 
 I made a quick benchmark on my computer (MacBook Air M1) by running `Pilot_Episode` with `GameEngineParams::enable_benchmark = true` and using the default draw policy (`t8::DrawPolicy::MEASURE_SELECT`). This was the results:
 ```
Goal FPS = 10000
Average FPS = 1810.25
# Full Redraws = 18
# Partial Redraws = 35124
```
So I would say, it's fairly fast.
