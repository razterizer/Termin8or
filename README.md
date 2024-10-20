# Termin8or

![GitHub License](https://img.shields.io/github/license/razterizer/Termin8or?color=blue)
![Static Badge](https://img.shields.io/badge/linkage-header_only-yellow)

[![build and test ubuntu](https://github.com/razterizer/Termin8or/actions/workflows/build-and-test-ubuntu.yml/badge.svg)](https://github.com/razterizer/Termin8or/actions/workflows/build-and-test-ubuntu.yml)

![Top Languages](https://img.shields.io/github/languages/top/razterizer/Termin8or)
![GitHub repo size](https://img.shields.io/github/repo-size/razterizer/Termin8or)
![Goto](https://img.shields.io/github/search/razterizer/Termin8or/goto)
![Commit Activity](https://img.shields.io/github/commit-activity/t/razterizer/Termin8or)
![Last Commit](https://img.shields.io/github/last-commit/razterizer/Termin8or?color=blue)
![Contributors](https://img.shields.io/github/contributors/razterizer/Termin8or?color=blue)
<!-- ![Languages](https://img.shields.io/github/languages/count/razterizer/Termin8or) -->

This is a cross-platform header-only curses-like lib.

Supported platforms (including but not limited to) are:
* Linux.
* MacOS.
* Windows.

## Showcases

![image](https://github.com/razterizer/Pilot_Episode/assets/32767250/9b596d5c-e128-4dba-8fb5-02e9b3d8f412)
![image](https://github.com/razterizer/Pilot_Episode/assets/32767250/abd78df7-7314-43a2-8ef6-0270dc979575)
<img width="568" alt="image" src="https://github.com/razterizer/SurgSim_Lite/assets/32767250/2d356a54-6117-4d6b-aeca-4a025e8ae372">
<img width="567" alt="image" src="https://github.com/razterizer/SurgSim_Lite/assets/32767250/47cf2c18-c9cf-46a5-9787-bfd3ad69540a">
<img width="568" alt="image" src="https://github.com/user-attachments/assets/c3075838-7f15-4679-862e-ce47780c9b39">
<img width="568" alt="image" src="https://github.com/user-attachments/assets/bfb04801-8969-418f-b5bd-0c24d37eb0b3">

## Header Files

* `ASCII_Fonts.h` : API for rendering text using FIGlet fonts and allows you to style your text with different colors. Supported FIGlet fonts are:
  * Larry3D.
  * SMSlant.
  * Avatar.
* `Color.h` : Contains colour definitions for the 16 colors that Termin8or (and the terminal) supports. There are also two transparency colours / modes which allows you to overlay text using the same colours that are already present in a given location in the screen buffer (see `ScreenHandler.h`).
* `Gradient.h` : Allows you to access a vector of given objects using a normalized (0 to 1) t parameter. Useful for particle systems and things like that where it is used for color gradients.
* `Drawing.h` : Features some drawing functions such as `bresenham::plot_line()`, `drawing::draw_box()`, `drawing::draw_box_textured()`, `drawing::draw_box_outline()` and `drawing::filled_circle_positions()`.
* `GameEngine.h` : A highly customizable buy easy to use game engine (or engine for any real-time terminal-based program).
* `Keyboard.h` : A keyboard handling API that is easy to use. Use class `StreamKeyboard` to poll key presses.
* `MessageHandler.h` : The `MessageHandler` class allows you to queue up messages of different severity levels and durations. Messages are displayed in a `ui::TextBox` in the middle of the screen.
* `ParticleSystem.h` : This ASCII-style particle system allows you to make cool real-time VFX such as liquids and fire-smoke. See ([`SurgSim_Lite`](https://github.com/razterizer/SurgSim_Lite) and [`Pilot_Episode`](https://github.com/razterizer/Pilot_Episode) for examples).
* `RC.h` : A struct representing the row and column position on the screen or in a texture or bounding box to mention a few.
* `Rectangle.h` : A rectangle struct that can be used for bounding boxes etc.
* `ScreenUtils.h` : A collection of functions for rendering dialogs and such:
  * Low-level functions: `clear_screen()`, `return_cursor()`, `restore_cursor()`, `gotorc()`.
  * `draw_frame()` : Draws a simple frame around your frame buffer.
  * `draw_game_over()`, `draw_you_won()` : Draws wavy banners in the FIGlet font Grafitti. Used by `GameEngine` if those features are enabled.
  
