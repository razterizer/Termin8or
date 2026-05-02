//
//  AnsiColor.h
//  Termin8or
//
//  Created by Rasmus Anthin on 2026-05-02.
//

#pragma once

#include "Color.h"
#include <string>
#include <vector>


namespace t8::ansi
{
  
  inline std::string colors_to_ansi_sgr_string(Color text_color, Color bg_color = Color16::Default)
  {
    std::string fg, bg;
    
    int text_color_idx = text_color.get_index();
    if (-1 <= text_color_idx) // Color16::Default = -1
    {
      fg = "\033[";
      if (text_color == Color16::Default)
        fg += "39";
      else if (0 <= text_color_idx && text_color_idx <= 7)
        fg += std::to_string(30 + text_color_idx);
      else if (8 <= text_color_idx && text_color_idx <= 15)
        fg += std::to_string(90 + (text_color_idx - 8));
      else if (16 <= text_color_idx)
        fg += "38;5;" + std::to_string(text_color_idx); // 256-color extended mode (6^3 rgb + 24 gray)
      
      fg += "m";
    }
    
    int bg_color_idx = bg_color.get_index();
    if (-1 <= bg_color_idx) // Color16::Default = -1
    {
      bg = "\033[";
      if (bg_color == Color16::Default)
        bg += "49";
      else if (0 <= bg_color_idx && bg_color_idx <= 7)
        bg += std::to_string(40 + bg_color_idx);
      else if (8 <= bg_color_idx && bg_color_idx <= 15)
        bg += std::to_string(100 + (bg_color_idx - 8));
      else if (16 <= bg_color_idx)
        bg += "48;5;" + std::to_string(bg_color_idx); // 256-color extended mode (6^3 rgb + 24 gray)
      
      bg += "m";
    }
    
    return fg + bg;
  }
  
  // 0   -> fg = Default, bg = Default
  // 39  -> fg = Default
  // 49  -> bg = Default
  // 31  -> fg = red
  // 44  -> bg = blue
  // 38;5;123 -> fg = color index 123
  // 48;5;123 -> bg = color index 123
  bool apply_ansi_sgr_params(const std::vector<int>& params,
                             Color& fg,
                             Color& bg,
                             Color default_fg = Color16::Default,
                             Color default_bg = Color16::Transparent2)
  {
    if (params.empty())
    {
      fg = default_fg;
      bg = default_bg;
      return true;
    }
    
    int rgb6_fg_state = 0;
    int rgb6_bg_state = 0;
    for (int p : params)
    {
      if (p == 0)
      {
        fg = default_fg;
        bg = default_bg;
      }
      else if (p == 39)
        fg = default_fg;
      else if (p == 49)
        bg = default_bg;
      else if (30 <= p && p <= 37)
        fg = Color { p - 30 };
      else if (90 <= p && p <= 97)
        fg = Color { p - 90 + 8 };
      else if (40 <= p && p <= 47)
        bg = Color { p - 40 };
      else if (100 <= p && p <= 107)
        bg = Color { p - 100 + 8 };
      else if (p == 38)
        rgb6_fg_state = 1;
      else if (p == 48)
        rgb6_bg_state = 1;
      else if (p == 5)
      {
        if (rgb6_fg_state == 1)
          rgb6_fg_state = 2;
        else if (rgb6_bg_state == 1)
          rgb6_bg_state = 2;
      }
      else if (rgb6_fg_state == 2)
      {
        fg = Color { p };
        rgb6_fg_state = 0;
      }
      else if (rgb6_bg_state == 2)
      {
        bg = Color { p };
        rgb6_bg_state = 0;
      }
    }
    return true;
  }
  
}
