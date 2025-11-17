#pragma once
#include "../screen/Color.h"

namespace t8x
{
  
  struct Pixel
  {
    using Color16 = t8::Color16;
  
    Pixel(char c, Color16 fg, Color16 bg,
          int rr = -1, int cc = -1, int ln_idx = -1, int ch_idx = -1, bool enabled = true)
      : ch(c), str(1, ch), fg_color(fg), bg_color(bg),
        r(rr), c(cc), r_orig(rr), c_orig(cc),
        line_idx(ln_idx), char_idx(ch_idx), active(enabled) { }
    char ch;
    std::string str;
    Color16 fg_color;
    Color16 bg_color;
    int r = -1;
    int c = -1;
    int r_orig = -1;
    int c_orig = -1;
    int line_idx = -1;
    int char_idx = -1;
    bool active = true;
  };
  
}
