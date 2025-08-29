#pragma once

struct Pixel
{
  Pixel(char c, Color fg, Color bg,
        int rr = -1, int cc = -1, int ln_idx = -1, int ch_idx = -1, bool enabled = true)
    : ch(c), str(1, ch), fg_color(fg), bg_color(bg),
      r(rr), c(cc), r_orig(rr), c_orig(cc),
      line_idx(ln_idx), char_idx(ch_idx), active(enabled) { }
  char ch;
  std::string str;
  Color fg_color;
  Color bg_color;
  int r = -1;
  int c = -1;
  int r_orig = -1;
  int c_orig = -1;
  int line_idx = -1;
  int char_idx = -1;
  bool active = true;
};
