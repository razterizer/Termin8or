# Termin8or

This is a header-only curses-like lib.

# Header Files

* `ASCII_Fonts.h` : API for rendering text using FIGlet fonts and allows you to style your text with different colors. Supported FIGlet fonts are:
  * Larry3D.
  * SMSlant.
  * Avatar.
* `Color.h` : Contains colour definitions for the 16 colors that Termin8or (and the terminal) supports. There are also two transparency colours / modes which allows you to overlay text using the same colours that are already present in a given location in the screen buffer (see `SpriteHandler.h`).
* `ColorGradient.h` : Allows you to access a vector of given colors using a normalized (0 to 1) t parameter. Useful for particle systems and things like that.
* `Drawing.h` : Features some drawing functions such as `bresenham::plot_line()`, `drawing::draw_box()`, `drawing::draw_box_textured()`, `drawing::draw_box_outline()` and `drawing::filled_circle_positions()`.