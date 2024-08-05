# Termin8or

This is a header-only curses-like lib.

# Header Files

* `ASCII_Fonts.h` : API for rendering text using FIGlet fonts and allows you to style your text with different colors. Supported FIGlet fonts are:
  * Larry3D.
  * SMSlant.
  * Avatar.
* `Color.h` : Contains colour definitions for the 16 colors that Termin8or (and the terminal) supports. There are also two transparency colours / modes which allows you to overlay text using the same colours that are already present in a given location in the screen buffer (see `SpriteHandler.h`). 