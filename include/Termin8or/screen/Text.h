#pragma once
#include "Color.h"
#include "ScreenCommandsBasic.h"
#include "../geom/RC.h"
#include <Core/System.h>
#include <Core/Utf8.h>
#include <string>
#include <vector>
#include <tuple>
#include <iostream>
#include <mutex>    // once_flag, call_once
#include <clocale>  // setlocale, LC_CTYPE
#include <cwchar>   // wcwidth (or <wchar.h>)
#ifdef _WIN32
#include <conio.h>
#ifndef NOMINMAX
#define NOMINMAX // Should fix the std::min()/max() and std::numeric_limits<T>::min()/max() compilation problems.
#endif
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

  namespace term
  {
    inline void init_locale()
    {
      static std::once_flag flag;
      std::call_once(flag, []()
      {
        std::setlocale(LC_CTYPE, "");
      });
    }
    
    inline int get_code_page()
    {
      return sys::is_windows_cmd() ? 437 : 65001;
    }
    
    // We assume single column and rely on encoder fallback.
    inline bool is_single_column(char32_t cp)
    {
      if (sys::is_windows_cmd())
      {
        // Classic Windows console: only single-byte glyphs are safe.
        return cp <= 0x7F;
      }
      
      if (cp > 0x10FFFF)
        return false;
      
      init_locale();
      wchar_t wc = static_cast<wchar_t>(cp);
      int w = wcwidth(wc);
      return w == 1; //w <= 1;
    }
    
    inline char32_t get_single_column_char32(char32_t cp)
    {
      return is_single_column(cp) ? cp : U'?';
    }
    
    inline std::string encode_single_width_glyph(char32_t preferred,
                                                 char32_t fallback = U'?')
    {
      char32_t cp = preferred;
      if (!is_single_column(cp))
        cp = fallback;
      if (!is_single_column(cp))
        cp = U'?';
        
      // Use 437 for cmd.exe for now as we only have a mapping from UTF-8 to CP437 atm.
      return utf8::encode_char32_codepage(cp, sys::is_windows_cmd() ? 437 : 65001);
    }
  }
  
  class Text
  {
  
    int code_page = 65001;
    
  public:
    Text()
    {
      if (sys::is_windows_cmd())
        code_page = 437;
    }
    
    static std::string get_color_string(Color text_color, Color bg_color = Color16::Default)
    {
      std::string fg, bg;
    
      int text_color_idx = text_color.get_index();
      if (-1 <= text_color_idx) // Color16::Default = -1
      {
        fg = "\033[";
        if (text_color == Color16::Default)
          fg += "39";
        else if (0 <= text_color_idx && text_color_idx <= 7)
          fg += std::to_string(text_color_idx + 30);
        else if (8 <= text_color_idx && text_color_idx <= 15)
          fg += std::to_string(text_color_idx + 82);
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
          bg += std::to_string(bg_color_idx + 40);
        else if (8 <= bg_color_idx && bg_color_idx <= 15)
          bg += std::to_string(bg_color_idx + 92);
        else if (16 <= bg_color_idx)
          bg += "48;5;" + std::to_string(bg_color_idx); // 256-color extended mode (6^3 rgb + 24 gray)
        
        bg += "m";
      }
      
      return fg + bg;
    }

    static int get_color_win_cmd(Color color)
    {
#ifdef _WIN32
      auto color16 = to_nearest_color16(color);
      int win_idx = get_color16_value_win(color16);
      return win_idx;
#endif
      return -1;
    }
    
    // For classic cmd.exe.
    static void set_color_win_cmd(Color text_color, Color bg_color = Color16::Default)
    {
#ifdef _WIN32
      int foreground = get_color_win_cmd(text_color);
      int background = 16 * get_color_win_cmd(bg_color);
      
      int color = foreground + background;
      
      SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
#endif
    }
    
    static void print(const std::string& text, Color text_color, Color bg_color = Color16::Default)
    {
      if (sys::is_windows_cmd())
      {
        set_color_win_cmd(text_color, bg_color);
        std::cout << text;
      }
      else
      {
        std::string output = get_color_string(text_color, bg_color) + text + "\033[0m";
        //printf("%s", output.c_str());
        std::cout << output;
      }
    }
    
    static void print_line(const std::string& text, Color text_color, Color bg_color = Color16::Default)
    {
      print(text, text_color, bg_color);
      //printf("\n");
      std::cout << "\n";
    }
    
    static void print_char(char c, Color text_color, Color bg_color = Color16::Default)
    {
      if (sys::is_windows_cmd())
      {
        set_color_win_cmd(text_color, bg_color);
        std::cout << c;
      }
      else
      {
        std::string output = get_color_string(text_color, bg_color) + c + "\033[0m";
        //printf("%s", output.c_str());
        std::cout << output;
      }
    }
    
    static void print_char(char32_t c, Color text_color, Color bg_color = Color16::Default)
    {
      if (sys::is_windows_cmd())
      {
        set_color_win_cmd(text_color, bg_color);
        std::cout << utf8::encode_char32_codepage(c, code_page);
      }
      else
      {
        std::string output = get_color_string(text_color, bg_color) + utf8::encode_char32_codepage(c, code_page) + "\033[0m";
        //printf("%s", output.c_str());
        std::cout << output;
      }
    }
    
    using ComplexString = std::vector<std::tuple<char32_t, Color, Color>>;
    
    void print_complex_sequential(const ComplexString& text)
    {
      if (sys::is_windows_cmd())
      {
#ifdef _WIN32
        HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
        SHORT currentRow = 0;
        
        std::vector<CHAR_INFO> lineBuffer;
        for (size_t i = 0; i < text.size(); ++i)
        {
          auto [ch, fg, bg] = text[i];
          
          if (ch == '\n')
          {
            if (!lineBuffer.empty())
            {
              COORD bufferSize = { (SHORT)lineBuffer.size(), 1 };
              COORD bufferCoord = { 0, 0 };
              SMALL_RECT writeRegion = { 0, currentRow, (SHORT)lineBuffer.size() - 1, currentRow };
              WriteConsoleOutput(hConsole, lineBuffer.data(), bufferSize, bufferCoord, &writeRegion);
              lineBuffer.clear();
            }
            currentRow++;
            continue;
          }
          
          CHAR_INFO ci {};
          ci.Char.AsciiChar = utf8::encode_char32_codepage(ch, code_page);
          int fgAttr = get_color_win_cmd(fg); if (fgAttr == -1) fgAttr = 7;
          int bgAttr = get_color_win_cmd(bg); if (bgAttr == -1) bgAttr = 0;
          ci.Attributes = fgAttr | (bgAttr << 4);
          lineBuffer.push_back(ci);
        }
#endif
      }
      else
      {
        size_t n = text.size();
        std::string output;
        for (size_t i = 0; i < n; ++i)
        {
          auto [c, fg_color, bg_color] = text[i];
          auto col_str = get_color_string(fg_color, c == '\n' ? Color16::Default : bg_color);
          output += col_str + utf8::encode_char32_codepage(c, code_page);
        }
        output += "\033[0m";
        //printf("%s", output.c_str());
        std::cout << output;
      }
    }
    
    struct ComplexStringChunk
    {
      RC pos;
      ComplexString text;
    };
    
    void print_complex_chunks(const std::vector<ComplexStringChunk>& chunk_vec)
    {
      if (sys::is_windows_cmd())
      {
#ifdef _WIN32
        HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
        
        for (const auto& chunk : chunk_vec)
        {
          COORD coord;
          coord.X = chunk.pos.c;
          coord.Y = chunk.pos.r;
          
          std::vector<CHAR_INFO> buffer(chunk.text.size());
          for (size_t i = 0; i < chunk.text.size(); ++i)
          {
            CHAR_INFO ci {};
            auto [ch, fg_color, bg_color] = chunk.text[i];
            ci.Char.AsciiChar = utf8::encode_char32_codepage(ch, code_page);
            ci.Attributes =
              get_color_win_cmd(fg_color) |
             (get_color_win_cmd(bg_color) << 4);
            buffer[i] = ci;
          }
          
          SMALL_RECT writeRegion;
          writeRegion.Left = coord.X;
          writeRegion.Top = coord.Y;
          writeRegion.Right = coord.X + (SHORT)chunk.text.size() - 1;
          writeRegion.Bottom = coord.Y;
          
          COORD bufferSize = { (SHORT)chunk.text.size(), 1 };
          COORD bufferCoord = { 0, 0 };
          
          WriteConsoleOutput(hConsole, buffer.data(), bufferSize, bufferCoord, &writeRegion);
        }
#endif
      }
      else
      {
        std::string output;
        for (const auto& chunk : chunk_vec)
        {
          output += get_gotorc_str(chunk.pos.r, chunk.pos.c);
          for (const auto& [ch, fg, bg] : chunk.text)
            output += get_color_string(fg, bg) + utf8::encode_char32_codepage(ch, code_page);
        }
        
        // Reset color.
        output += "\033[0m";
        std::cout << output;
      }
    }
    
    void print_reset() const
    {
      if (sys::is_windows_cmd())
        set_color_win_cmd(Color16::White, Color16::Black);
      else
      {
        //printf("%s", "\033[0m");
        std::cout << "\033[0m";
      }
    }
    
    inline int get_code_page() const noexcept { return code_page; }
  };
  
}
