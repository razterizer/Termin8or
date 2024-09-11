# Termin8or

![GitHub License](https://img.shields.io/github/license/razterizer/Termin8or?color=blue)

This is a cross-platform header-only curses-like lib.

## Showcases

![image](https://github.com/razterizer/Pilot_Episode/assets/32767250/abd78df7-7314-43a2-8ef6-0270dc979575)
<img width="567" alt="image" src="https://github.com/razterizer/SurgSim_Lite/assets/32767250/47cf2c18-c9cf-46a5-9787-bfd3ad69540a">

## Header Files

* `ASCII_Fonts.h` : API for rendering text using FIGlet fonts and allows you to style your text with different colors. Supported FIGlet fonts are:
  * Larry3D.
  * SMSlant.
  * Avatar.
* `Color.h` : Contains colour definitions for the 16 colors that Termin8or (and the terminal) supports. There are also two transparency colours / modes which allows you to overlay text using the same colours that are already present in a given location in the screen buffer (see `SpriteHandler.h`).
* `Gradient.h` : Allows you to access a vector of given objects using a normalized (0 to 1) t parameter. Useful for particle systems and things like that where it is used for color gradients.
* `Drawing.h` : Features some drawing functions such as `bresenham::plot_line()`, `drawing::draw_box()`, `drawing::draw_box_textured()`, `drawing::draw_box_outline()` and `drawing::filled_circle_positions()`.
* `GameEngine.h` : A highly customizable buy easy to use game engine (or engine for any real-time terminal-based program).
* `Keyboard.h` : A keyboard handling API that is easy to use. Use class `StreamKeyboard` to poll key presses.
* `MessageHandler.h` : The `MessageHandler` class allows you to queue up messages of different severity levels and durations. Messages are displayed in a `ui::TextBox` in the middle of the screen.
* `ParticleSystem.h` : This ASCII-style particle system allows you to make cool real-time VFX such as liquids and fire-smoke. See ([`SurgSim_Lite`](https://github.com/razterizer/SurgSim_Lite) and [`Pilot_Episode`](https://github.com/razterizer/Pilot_Episode) for examples).
* `RC.h` : A struct representing the row and column position on the screen or in a texture or bounding box to mention a few.
* `Rectangle.h` : A rectangle struct that can be used for bounding boxes etc.
* `Screen.h` : A collection of functions for rendering dialogs and such:
  * Low-level functions: `clear_screen()`, `return_cursor()`, `restore_cursor()`, `gotorc()`.
  * `draw_frame()` : Draws a simple frame around your frame buffer.
  * `draw_game_over()`, `draw_you_won()` : Draws wavy banners in the FIGlet font Grafitti. Used by `GameEngine` if those features are enabled.
  