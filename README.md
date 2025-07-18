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

## Sprite System

### Bitmap Sprites

![ubuntu_elli1an7bg_360](https://github.com/user-attachments/assets/19d3a6d6-b6f1-46a6-a80f-bcc15e202404)

Attaching the spaceship to a rigid body object:

![bitmap_sprite_dynamics](https://github.com/user-attachments/assets/74bfb3a8-51fe-4ffd-a06f-8792a476febb)

### Vector Sprites

![ubuntu_vhurwusqie_360](https://github.com/user-attachments/assets/35a03da6-d37d-479c-b84e-10891ac50825)

### Dynamics / Collision System

<!-- ![Kapture 2024-10-31 at 14 08 03](https://github.com/user-attachments/assets/2cd50164-db0f-4672-93e4-e0c508108079) -->
![vector_sprite_collisions](https://github.com/user-attachments/assets/0d562d19-059d-4af8-bb12-38578b5d1471)

Debug-drawing the broad-phase (AABB BVH) and the narrow-phase (character/character overlap tests) collision detection:

![vector_sprite_collisions_dbg](https://github.com/user-attachments/assets/20e3ba2c-d2d4-463c-939e-2f22b2a74c7d)

---
## Header Files

* `AABB` : A templetized (int, float) AABB class that can return a `Rectangle` object if needed. Used in `CollisionHandler` for broad-phase detection.
* `ASCII_Fonts.h` : API for rendering text using FIGlet fonts and allows you to style your text with different colors. Supported FIGlet fonts are:
  * Larry3D.
  * SMSlant.
  * Avatar.
* `Color.h` : Contains colour definitions for the 16 colors that Termin8or (and the terminal) supports. There are also two transparency colours / modes which allows you to overlay text using the same colours that are already present in a given location in the screen buffer (see `ScreenHandler.h`).
* `Gradient.h` : Allows you to access a vector of given objects using a normalized (0 to 1) t parameter. Useful for particle systems and things like that where it is used for color gradients.
* `Drawing.h` : Features some drawing functions such as `bresenham::plot_line()`, `drawing::draw_box()`, `drawing::draw_box_textured()`, `drawing::draw_box_outline()` and `drawing::filled_circle_positions()`.
* `GameEngine.h` : A highly customizable buy easy to use game engine (or engine for any real-time terminal-based program).
* `Keyboard.h` : A keyboard handling API that is easy to use. Use class `StreamKeyboard` to poll key presses. Function `readKey()` scans keypresses in an un-blocked manner and returns a struct `KeyPressDataPair` containing two objects of type `KeyPressData`; `transient` and `held`. The former being the raw key presses and the latter being the raw key presses buffered in a buffer that has a size proportional to the FPS of the application. These two modes have their pros and cons: Transient mode is more accurate but cannot capture held key presses, held mode on the other hand is great at capturing held keys but due to buffering, it suffers from minor inaccurracies.
* `Logging.h` : Allows you to record the current random seed, frame numbers and respective keypresses in your program and then replay it. This makes finding runtime bugs a breeze.
* `LineData.h` : a struct that helps to convert bigger chunks of strings / colors along a single line of text to individual "pixels" that can be "streamed" to the `ScreenHandler`. This will likely be deprecated in the future in favour of more recent tools such as sprites.
* `Pixel.h` : a struct containing data such as character (or std::string of a single character), foreground color, background color (r,c) position (and original position), line index to `LineData` object and char index to within a `LineData` object and an enabled flag. This will likely be deprecated in the future in favour of more recent tools such as sprites.
* `MessageHandler.h` : The `MessageHandler` class allows you to queue up messages of different severity levels and durations. Messages are displayed in a `ui::TextBox` in the middle of the screen.
* `ParticleSystem.h` : This ASCII-style particle system allows you to make cool real-time VFX such as liquids and fire-smoke. See ([`SurgSim_Lite`](https://github.com/razterizer/SurgSim_Lite),  [`Pilot_Episode`](https://github.com/razterizer/Pilot_Episode) and [`DungGine`](https://github.com/razterizer/DungGine) for examples).
* `RC.h` : A struct representing the row and column position on the screen or in a texture or bounding box to mention a few.
* `Rectangle.h` : A rectangle struct that can be used for bounding boxes etc.
* `ScreenUtils.h` : A collection of functions for rendering dialogs and such:
  * Low-level functions: `clear_screen()`, `return_cursor()`, `restore_cursor()`, `hide_cursor()`, `show_cursor()`, `gotorc()`.
  * Medium-level functions: `get_terminal_window_size()`, `resize_terminal_window()`, `save_terminal_colors()`, `restore_terminal_colors()`.
  * High-level functions: `begin_screen()`, `end_screen()`. These take care of color restoration, clearing screen, hiding the cursor etc, except for the resizing of the terminal window.
  * `draw_frame()` : Draws a simple frame around your frame buffer.
  * `draw_game_over()`, `draw_you_won()` : Draws wavy banners in the FIGlet font Grafitti. Used by `GameEngine` if those features are enabled.
* `Text.h` : `Text` handles text output and translates Color enum items to corresponding color values depending on platform compiled. Is also responsible for creating the appropriate ANDI escape sequences for the TTY and such.
* `Styles.h` : `Style` and its derivatives are just fancy structs grouping a foreground colour with a background colour.
* `Texture.h`: The `Texture` struct allows you to save and load text-based texture in text format to and from disk. There are four parts in a texture: characters, foreground colors, background colors and material IDs. Header also contains texture element `Textel` (humouristic portmantau of "text" and "texel" suggested by my dear colleague Göran Wallgren).
* `ScreenHandler.h` : Contains the screen buffers (char / fg-color / bg-color) and manages transparency etc. It outputs the contents to the terminal via a privatly owned `Text` object.
* `SpriteHandler.h` : Manages bitmap sprites (`BitmapSprite`) and vector sprites (`VectorSprite`). A sprite can be controlled programmatically or be attached to a `RigidBody` object. Both sprite classes support sprite animations.
* `UI.h` : Contains structs such as `TextBox` and `TextBoxDebug` to make it easy to display info on the screen and such. `MessageHandler` uses `TextBox`.
* `Dynamics` : Headers concerning rigid body physics.
  * `RigidBody.h` : A class that represents a rigid body. You attach a sprite to it if you want the sprite to be physically dynamic. The sprite also determines the "pixels" that make out the collison surface.
  * `DynamicsSystem.h` : A class that governs the dynamical motions of the rigid bodies that are created from it.
  * `CollisionHandler.h` : A class that governs collison detection and collison response between the rigid bodies created via the `DynamicsSystem` class. Broad-phase uses an AABB BVH and narrow-phase checks overlaps beteen sprite characters of a specified material index. Collision response uses an impulse equation as a function of the velocities of the two bodies and their collision normals.
