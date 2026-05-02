//
//  AnsiColor.h
//  Termin8or
//
//  Created by Rasmus Anthin on 2026-05-02.
//

#pragma once

#include "Color.h"
#include <string>


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
  
}
