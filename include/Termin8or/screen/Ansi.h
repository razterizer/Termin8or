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
                                    bool& bright_fg,
                                    Color default_fg = Color16::Default,
                                    Color default_bg = Color16::Transparent2)
  {
    if (params.empty())
    {
      fg = default_fg;
      bg = default_bg;
      bright_fg = false;
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
        bright_fg = false;
      }
      else if (p == 1)
      {
        bright_fg = true;
        int fg_idx = fg.get_index();
        if (0 <= fg_idx && fg_idx <= 7)
          fg = Color { fg_idx + 8 };
      }
      else if (p == 22)
      {
        bright_fg = false;
        int fg_idx = fg.get_index();
        if (8 <= fg_idx && fg_idx <= 15)
          fg = Color { fg_idx - 8 };
      }
      else if (p == 39)
      {
        fg = default_fg;
        bright_fg = false;
      }
      else if (p == 49)
        bg = default_bg;
      else if (30 <= p && p <= 37)
        fg = Color { p - 30 + (bright_fg ? 8 : 0) };
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
  
  struct AnsiCsiSequence
  {
    std::vector<int> params;
    char command = '\0';
  };
  
  inline bool parse_ansi_csi_sequence(const std::string& str,
                                      int& pos,
                                      AnsiCsiSequence& seq)
  {
    seq.params.clear();
    seq.command = '\0';
    
    int i = pos;
    int str_len = str::lenI(str);
    if (i + 1 >= str_len || str[i] != '\033' || str[i + 1] != '[')
      return false;
    
    i += 2;
    std::string token;
    
    while (i < str_len)
    {
      char ch = str[i];
      
      if (str::is_digit(ch))
        token.push_back(ch);
      else if (ch == ';')
      {
        seq.params.emplace_back(token.empty() ? 0 : std::stoi(token));
        token.clear();
      }
      else if ('@' <= ch && ch <= '~') // Final sequence byte found somewhere in this range.
      {
        if (!token.empty())
          seq.params.emplace_back(std::stoi(token));
        
        seq.command = ch;
        pos = i + 1;
        return true;
      }
      else
        return false;
      
      ++i;
    }
    
    return false;
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
    AnsiCsiSequence ansi_seq;
    if (!parse_ansi_csi_sequence(str, pos, ansi_seq))
      return false;
    if (ansi_seq.command == 'm')
    {
      params = ansi_seq.params;
      return true;
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
    AnsiCsiSequence ansi_seq;
    if (!parse_ansi_csi_sequence(str, pos, ansi_seq))
      return false;
    auto dir = ansi_seq.command;
    if (dir == 'A' || dir == 'B' || dir == 'C' || dir == 'D')
    {
      direction = dir;
      count = ansi_seq.params.empty() ? 1 : ansi_seq.params[0];
      return true;
    }
    return false;
  }
  
  inline bool parse_ansi_erase(const std::string& str,
                               int& pos,
                               char& target,
                               int& mode)
  {
    AnsiCsiSequence ansi_seq;
    if (!parse_ansi_csi_sequence(str, pos, ansi_seq))
      return false;
    auto trg = ansi_seq.command;
    if (trg == 'J' || trg == 'K')
    {
      target = trg;
      mode = ansi_seq.params.empty() ? 0 : ansi_seq.params[0];
      return true;
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
    AnsiCsiSequence ansi_seq;
    if (!parse_ansi_csi_sequence(str, pos, ansi_seq))
      return false;
    auto cmd = ansi_seq.command;
    if (cmd == 'H' || cmd == 'f')
    {
      row = ansi_seq.params.empty() || ansi_seq.params[0] <= 0 ? 1 : ansi_seq.params[0];
      col = ansi_seq.params.size() < 2 || ansi_seq.params[1] <= 0 ? 1 : ansi_seq.params[1];
      return true;
    }
    return false;
  }
  
  // ESC[s -> save cursor position.
  // ESC[u -> restore cursor position.
  inline bool parse_ansi_cursor_save_restore(const std::string& str,
                                             int& pos,
                                             char& command)
  {
    AnsiCsiSequence ansi_seq;
    if (!parse_ansi_csi_sequence(str, pos, ansi_seq))
      return false;
    auto cmd = ansi_seq.command;
    if (cmd == 's' || cmd == 'u')
    {
      command = cmd;
      return true;
    }
    return false;
  }
  
}
