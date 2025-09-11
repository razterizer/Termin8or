#pragma once
#include "Color.h"
#include "ScreenCommandsBasic.h"
#include "../geom/RC.h"
#include <string>
#include <vector>
#include <tuple>
#include <iostream>
#ifdef _WIN32
#include <conio.h>
#define NOMINMAX // Should fix the std::min()/max() and std::numeric_limits<T>::min()/max() compilation problems.
#include <Windows.h>
#endif

//--------------------------+------------+------------+--------+--------+
// Color                    | Foreground | Background | FG Win | BG Win |
//--------------------------+------------+------------+--------+--------+
// Default                  | \033[39m   | \033[49m   |  N/A   |  N/A   |
// Black                    | \033[30m   | \033[40m   |  0     |  0     |
// Dark red                 | \033[31m   | \033[41m   |  4     |  64    |
// Dark green               | \033[32m   | \033[42m   |  2     |  32    |
// Dark yellow (Orange-ish) | \033[33m   | \033[43m   |  6     |  96    |
// Dark blue                | \033[34m   | \033[44m   |  1     |  16    |
// Dark magenta             | \033[35m   | \033[45m   |  5     |  80    |
// Dark cyan                | \033[36m   | \033[46m   |  3     |  48    |
// Light gray               | \033[37m   | \033[47m   |  7     |  112   |
// Dark gray                | \033[90m   | \033[100m  |  8     |  128   |
// Red                      | \033[91m   | \033[101m  |  12    |  192   |
// Green                    | \033[92m   | \033[102m  |  10    |  160   |
// Yellow                   | \033[93m   | \033[103m  |  14    |  224   |
// Blue                     | \033[94m   | \033[104m  |  9     |  144   |
// Magenta                  | \033[95m   | \033[105m  |  13    |  208   |
// Cyan                     | \033[96m   | \033[106m  |  11    |  176   |
// White                    | \033[97m   | \033[107m  |  15    |  240   |
// -------------------------+------------+------------+--------+--------+
//
// Reset : \033[0m

namespace t8
{
  
  class Text
  {
    
    template<typename EnumType>
    constexpr int to_int(EnumType e) const noexcept
    {
      return static_cast<int>(e);
    }
    
  public:
    Text()
    {
      std::ios_base::sync_with_stdio(false);
    }
    
    std::string get_color_string(Color text_color, Color bg_color = Color::Default) const
    {
      if (bg_color == Color::Transparent || bg_color == Color::Transparent2)
        bg_color = Color::Default;
      std::string fg = "\033[";
      std::string bg = "\033[";
      if (text_color == Color::Default)
        fg += "39";
      else if (Color::Default < text_color && text_color <= Color::LightGray)
        fg += std::to_string(to_int(text_color) + 29);
      else
        fg += std::to_string(to_int(text_color) + 81);
      
      if (bg_color == Color::Default)
        bg += "49";
      else if (Color::Default < bg_color && bg_color <= Color::LightGray)
        bg += std::to_string(to_int(bg_color) + 39);
      else
        bg += std::to_string(to_int(bg_color) + 91);
      
      fg += "m";
      bg += "m";
      return fg + bg;
    }
    
    
    void set_color_win(Color text_color, Color bg_color = Color::Default) const
    {
#ifdef _WIN32
      int foreground = get_color_value_win(text_color);
      if (foreground == -1)
        foreground = get_color_value_win(Color::White);
      int background = 16 * get_color_value_win(bg_color);
      if (background == -1)
        background = 16 * get_color_value_win(Color::Black);
      
      int color = static_cast<int>(foreground) + static_cast<int>(background);
      
      SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
#endif
    }
    
    void print(const std::string& text, Color text_color, Color bg_color = Color::Default) const
    {
#ifdef _WIN32
      set_color_win(text_color, bg_color);
      std::cout << text;
#else
      std::string output = get_color_string(text_color, bg_color) + text + "\033[0m";
      //printf("%s", output.c_str());
      std::cout << output;
#endif
    }
    
    void print_line(const std::string& text, Color text_color, Color bg_color = Color::Default) const
    {
      print(text, text_color, bg_color);
      //printf("\n");
      std::cout << "\n";
    }
    
    void print_char(char c, Color text_color, Color bg_color = Color::Default) const
    {
#ifdef _WIN32
      set_color_win(text_color, bg_color);
      std::cout << c;
#else
      std::string output = get_color_string(text_color, bg_color) + c;
      //printf("%s", output.c_str());
      std::cout << output;
#endif
    }
    
    using ComplexString = std::vector<std::tuple<char, Color, Color>>;
    
    void print_complex(const ComplexString& text)
    {
      size_t n = text.size();
      std::string output;
#ifdef _WIN32
      auto fg_color_prev = Color::Default;
      auto bg_color_prev = Color::Default;
      char c_prev = -1;
#endif
      for (size_t i = 0; i < n; ++i)
      {
        const auto& ti = text[i];
        char c = std::get<0>(ti);
        auto fg_color = std::get<1>(ti);
        auto bg_color = std::get<2>(ti);
#ifdef _WIN32
        if (c_prev != -1)
          output += c_prev;
        if (fg_color != fg_color_prev || bg_color != bg_color_prev)
        {
          set_color_win(fg_color_prev, bg_color_prev);
          std::cout << output;
          output = "";
        }
        c_prev = c;
        fg_color_prev = fg_color;
        bg_color_prev = bg_color;
#else
        auto col_str = get_color_string(fg_color, c == '\n' ? Color::Default : bg_color);
        std::string char_str(1, c);
        output += col_str + char_str;
#endif
      }
#ifndef _WIN32
      output += "\033[0m";
      //printf("%s", output.c_str());
      std::cout << output;
#endif
    }
    
    struct ComplexStringChunk
    {
      RC pos;
      ComplexString text;
    };
    
    void print_complex(const std::vector<ComplexStringChunk>& chunk_vec)
    {
      for (const auto& chunk : chunk_vec)
      {
        gotorc(chunk.pos.r, chunk.pos.c);
        print_complex(chunk.text);
      }
    }
    
    void print_reset() const
    {
#ifdef _WIN32
      set_color_win(Color::White, Color::Black);
#else
      //printf("%s", "\033[0m");
      std::cout << "\033[0m";
#endif
    }
  };
  
}
