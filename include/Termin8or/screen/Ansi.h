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
  inline bool apply_ansi_sgr_params(const std::vector<int>& params,
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
    
    const int N = stlutils::sizeI(params);
    for (int i = 0; i < N; ++i)
    {
      int p = params[i];
      
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
      else if (p == 38 && i + 1 < N && params[i + 1] == 5)
      {
        int idx = i + 2 < N ? params[i + 2] : -1;
        if (0 <= idx && idx <= 255)
          fg = Color { idx };
        i += 2;
      }
      else if (p == 48 && i + 1 < N && params[i + 1] == 5)
      {
        int idx = i + 2 < N ? params[i + 2] : -1;
        if (0 <= idx && idx <= 255)
          bg = Color { idx };
        i += 2;
      }
    }
    return true;
  }
  
  // ESC[m         -> params empty, apply_ansi_sgr_params() resets
  // ESC[0m        -> {0}
  // ESC[31m       -> {31}
  // ESC[31;44m    -> {31, 44}
  // ESC[38;5;123m -> {38, 5, 123}
  inline bool parse_ansi_sgr_params(const std::string& str,
                                    int& pos,
                                    std::vector<int>& params)
  {
    params.clear();
    
    int i = pos;
    int str_len = str::lenI(str);
    if (i + 1 >= str_len)
      return false;
    
    if (str[i] != '\033' || str[i + 1] != '[')
      return false;
    
    // I could've used str::tokenize() here, but this manual parser is more robust.

    i += 2;
    
    std::string token;
    while (i < str_len)
    {
      char ch = str[i];
      
      if (str::is_digit(ch))
        token.push_back(ch);
      else if (ch == ';')
      {
        params.emplace_back(token.empty() ? 0 : std::stoi(token));
        token.clear();
      }
      else if (ch == 'm')
      {
        if (!token.empty())
          params.emplace_back(std::stoi(token));
        
        pos = i + 1;
        return true;
      }
      else
        return false;
      
      ++i;
    }
    
    return false;
  }
  
  // ESC[A  -> move cursor up by 1 visible cell row.
  // ESC[nA -> move cursor up by n visible cell rows.
  // ESC[B  -> move cursor down by 1 visible cell row.
  // ESC[nB -> move cursor down by n visible cell rows.
  // ESC[C  -> move cursor forward by 1 visible cell.
  // ESC[nC -> move cursor forward by n visible cells.
  // ESC[D  -> move cursor backward by 1 visible cell.
  // ESC[nD -> move cursor backward by n visible cells.
  inline bool parse_ansi_cursor_move(const std::string& str,
                                     int& pos,
                                     char& direction,
                                     int& count)
  {
    int i = pos;
    int str_len = str::lenI(str);
    if (i + 1 >= str_len)
      return false;
    
    if (str[i] != '\033' || str[i + 1] != '[')
      return false;
    
    i += 2;
    
    std::string token;
    while (i < str_len)
    {
      char ch = str[i];
      
      if (str::is_digit(ch))
        token.push_back(ch);
      else if (ch == 'A' || ch == 'B' || ch == 'C' || ch == 'D')
      {
        direction = ch;
        count = token.empty() ? 1 : std::stoi(token);
        pos = i + 1;
        return true;
      }
      else
        return false;
      
      ++i;
    }
    
    return false;
  }
  
  inline bool parse_ansi_erase(const std::string& str,
                               int& pos,
                               char& target,
                               int& mode)
  {
    int i = pos;
    int str_len = str::lenI(str);
    if (i + 1 >= str_len)
      return false;
    
    if (str[i] != '\033' || str[i + 1] != '[')
      return false;
    
    i += 2;
    
    std::string token;
    while (i < str_len)
    {
      char ch = str[i];
      
      if (str::is_digit(ch))
        token.push_back(ch);
      else if (ch == 'J' || ch == 'K')
      {
        target = ch;
        mode = token.empty() ? 0 : std::stoi(token);
        pos = i + 1;
        return true;
      }
      else
        return false;
      
      ++i;
    }
    
    return false;
  }
  
  // ESC[H       -> move cursor to row 1, col 1.
  // ESC[row;colH -> move cursor to row, col. Coordinates are 1-based.
  // ESC[f       -> same as ESC[H.
  // ESC[row;colf -> same as ESC[row;colH.
  inline bool parse_ansi_cursor_position(const std::string& str,
                                         int& pos,
                                         int& row,
                                         int& col)
  {
    int i = pos;
    int str_len = str::lenI(str);
    if (i + 1 >= str_len)
      return false;
    
    if (str[i] != '\033' || str[i + 1] != '[')
      return false;
    
    i += 2;
    
    std::vector<int> params;
    std::string token;
    while (i < str_len)
    {
      char ch = str[i];
      
      if (str::is_digit(ch))
        token.push_back(ch);
      else if (ch == ';')
      {
        params.emplace_back(token.empty() ? 0 : std::stoi(token));
        token.clear();
      }
      else if (ch == 'H' || ch == 'f')
      {
        if (!token.empty())
          params.emplace_back(std::stoi(token));
        
        row = params.empty() || params[0] <= 0 ? 1 : params[0];
        col = params.size() < 2 || params[1] <= 0 ? 1 : params[1];
        pos = i + 1;
        return true;
      }
      else
        return false;
      
      ++i;
    }
    
    return false;
  }
  
}
