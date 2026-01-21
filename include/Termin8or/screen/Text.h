#pragma once
#include "Color.h"
#include "ScreenCommandsBasic.h"
#include "Glyph.h"
#include "../geom/RC.h"
#include <Core/System.h>
#include <Core/Utf8.h>
#include <Core/Term.h>
#include <string>
#include <vector>
#include <tuple>
#include <iostream>
#ifdef _WIN32
#include <conio.h>
#ifndef NOMINMAX
#define NOMINMAX // Should fix the std::min()/max() and std::numeric_limits<T>::min()/max() compilation problems.
#endif
#include <windows.h>
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
    ::term::TermMode m_term_mode;
    
#ifdef _WIN32
    template <auto WriteFn>
    static auto make_flush(HANDLE hConsole,
                    std::vector<CHAR_INFO>& lineBuffer,
                    SHORT& currentRow)
    {
      return [&]()
      {
        if (lineBuffer.empty())
          return;
        
        COORD bufferSize  = { static_cast<SHORT>(lineBuffer.size()), 1 };
        COORD bufferCoord = { 0, 0 };
        SMALL_RECT writeRegion = { 0, currentRow, static_cast<SHORT>(lineBuffer.size()) - 1, currentRow };
        
        WriteFn(hConsole, lineBuffer.data(), bufferSize, bufferCoord, &writeRegion);
        lineBuffer.clear();
      };
    }
#endif
    
  public:
    Text() = default;
    
    void init_terminal_mode()
    {
      m_term_mode = ::term::init_terminal_mode(65001);
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
    
#ifdef _WIN32
    static int get_color_win_cmd(Color color)
    {
      auto color16 = to_nearest_color16(color);
      int win_idx = get_color16_value_win(color16);
      return win_idx;
    }

    static WORD get_style_win_cmd(Color fg, Color bg)
    {
      int fg_val = get_color_win_cmd(fg);
      if (fg_val == -1)
        fg_val= 7; // light gray
      int bg_val = get_color_win_cmd(bg);
      if (bg_val == -1)
        bg_val = 0; // black
      return static_cast<WORD>(fg_val | (bg_val << 4));
    }
#endif
    
    // For classic cmd.exe.
    static void set_color_win_cmd(Color text_color, Color bg_color = Color16::Default)
    {
#ifdef _WIN32
      SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), get_style_win_cmd(text_color, bg_color));
#endif
    }
    
    void print(const std::string& text, Color text_color, Color bg_color = Color16::Default) const
    {
#ifdef _WIN32
      if (!::term::use_ansi_colors(m_term_mode))
      {
        set_color_win_cmd(text_color, bg_color);
        ::term::emit_text(m_term_mode, text);
        return;
      }
#endif
      
      std::string output;
      output.reserve(text.size() + 32);
      output += get_color_string(text_color, bg_color);
      output += text;
      output += "\033[0m";
      ::term::emit_text(m_term_mode, output);
    }

    
    void print_line(const std::string& text, Color text_color, Color bg_color = Color16::Default) const
    {
      print(text, text_color, bg_color);
      ::term::emit_text(m_term_mode, "\n");
    }
    
    void print_char(char c, Color text_color, Color bg_color = Color16::Default) const
    {
#ifdef _WIN32
      if (!::term::use_ansi_colors(m_term_mode))
      {
        // Legacy console: use WinAPI colors, then emit the byte.
        set_color_win_cmd(text_color, bg_color);
        ::term::emit_text(m_term_mode, std::string_view(&c, 1));
        return;
      }
#endif
      
      // ANSI-capable: build escape + char + reset.
      std::string output;
      output.reserve(32);
      output += get_color_string(text_color, bg_color);
      output.push_back(c);
      output += "\033[0m";
      ::term::emit_text(m_term_mode, output);
    }
    
    void print_char(char32_t c, Color text_color, Color bg_color = Color16::Default) const
    {
      std::string glyph = utf8::encode_char32_utf8(c);
      
#ifdef _WIN32
      if (!::term::use_ansi_colors(m_term_mode))
      {
        set_color_win_cmd(text_color, bg_color);
        ::term::emit_text(m_term_mode, glyph);
        return;
      }
#endif
      
      std::string output;
      output.reserve(glyph.size() + 32);
      output += get_color_string(text_color, bg_color);
      output += glyph;
      output += "\033[0m";
      ::term::emit_text(m_term_mode, output);
    }
    
    template<typename CharT>
    using ComplexString = std::vector<std::tuple<CharT, Color, Color>>;
    
    template<typename CharT>
    void print_complex_sequential(const ComplexString<CharT>& text)
    {
      constexpr auto assert_on_template_arg = []()
      {
        static_assert(std::is_same_v<CharT, char> || std::is_same_v<CharT, char32_t>,
                  "ERROR in Text::print_complex_sequential(): unsupported CharT!");
      };
    
#ifdef _WIN32
      if (!::term::use_ansi_colors(m_term_mode))
      {
        HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
        SHORT currentRow = 0;
        
        std::vector<CHAR_INFO> lineBuffer;
        lineBuffer.reserve(text.size());
        
        auto flushA = make_flush<WriteConsoleOutputA>(hConsole, lineBuffer, currentRow);
        auto flushW = make_flush<WriteConsoleOutputW>(hConsole, lineBuffer, currentRow);
        
        for (auto [ch, fg, bg] : text)
        {
          if constexpr (std::is_same_v<CharT, char>)
          {
            if (ch == '\n')
            {
              flushA();
              ++currentRow;
              continue;
            }
            
            CHAR_INFO ci {};
            ci.Char.AsciiChar = static_cast<unsigned char>(ch); // Safe for ASCII/CP437-ish bytes.
            ci.Attributes = get_style_win_cmd(fg, bg);
            lineBuffer.push_back(ci);
          }
          else if constexpr (std::is_same_v<CharT, char32_t>)
          {
            if (ch == U'\n')
            {
              flushW();
              ++currentRow;
              continue;
            }
            
            CHAR_INFO ci{};
            char32_t cp = ch;
            if (cp > 0xFFFF) cp = U'?';
            ci.Char.UnicodeChar = static_cast<wchar_t>(cp); // BMP assumed.
            ci.Attributes = get_style_win_cmd(fg, bg);
            lineBuffer.push_back(ci);
          }
          else
            assert_on_template_arg();
        }
        
        if constexpr (std::is_same_v<CharT, char>)
          flushA();
        else if constexpr (std::is_same_v<CharT, char32_t>)
          flushW();
        else
          assert_on_template_arg();
        return;
      }
#endif
      
      // ANSI-capable path (all platforms).
      std::string output;
      output.reserve(text.size() * 8); // Rough estimate.
      
      for (const auto& [c, fg_color, bg_color] : text)
      {
        const bool is_nl = (c == static_cast<CharT>('\n'));
        output += get_color_string(fg_color, is_nl ? Color16::Default : bg_color);
        
        if constexpr (std::is_same_v<CharT, char>)
          output.push_back(c);
        else if constexpr (std::is_same_v<CharT, char32_t>)
          output += utf8::encode_char32_utf8(c);
        else
          assert_on_template_arg();
      }
      
      output += "\033[0m";
      ::term::emit_text(m_term_mode, output);
    }

    
    template<typename CharT>
    struct ComplexStringChunk
    {
      RC pos;
      ComplexString<CharT> text;
    };
    
    template<typename CharT>
    void print_complex_chunks(const std::vector<ComplexStringChunk<CharT>>& chunk_vec)
    {
#ifdef _WIN32
      if (!::term::use_ansi_colors(m_term_mode))
      {
        HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
        
        for (const auto& chunk : chunk_vec)
        {
          COORD coord
          {
            static_cast<SHORT>(chunk.pos.c),
            static_cast<SHORT>(chunk.pos.r)
          };
          
          std::vector<CHAR_INFO> buffer(chunk.text.size());
          
          for (size_t i = 0; i < chunk.text.size(); ++i)
          {
            auto [ch, fg, bg] = chunk.text[i];
          
            CHAR_INFO ci {};
            if constexpr (std::is_same_v<CharT, char>)
              ci.Char.AsciiChar = static_cast<unsigned char>(ch);
            else if constexpr (std::is_same_v<CharT, char32_t>)
            {
              char32_t cp = ch;
              if (cp > 0xFFFF) cp = U'?';
              ci.Char.UnicodeChar = static_cast<wchar_t>(cp); // BMP assumed.
            }
            else
              static_assert(std::is_same_v<CharT, char> || std::is_same_v<CharT, char32_t>,
                  "ERROR in Text::print_complex_chunks(): unsupported CharT!");
            
            ci.Attributes = get_style_win_cmd(fg, bg);
            buffer[i] = ci;
          }
          
          SMALL_RECT writeRegion;
          writeRegion.Left   = coord.X;
          writeRegion.Top    = coord.Y;
          writeRegion.Right  = coord.X + static_cast<SHORT>(chunk.text.size()) - 1;
          writeRegion.Bottom = coord.Y;
          
          COORD bufferSize  = { static_cast<SHORT>(chunk.text.size()), 1 };
          COORD bufferCoord = { 0, 0 };
          
          if constexpr (std::is_same_v<CharT, char>)
            WriteConsoleOutputA(hConsole, buffer.data(), bufferSize, bufferCoord, &writeRegion);
          else if constexpr (std::is_same_v<CharT, char32_t>)
            WriteConsoleOutputW(hConsole, buffer.data(), bufferSize, bufferCoord, &writeRegion);
        }
        return;
      }
#endif
      
      // ANSI-capable path (all platforms).
      std::string output;
      output.reserve(chunk_vec.size() * 32); // Rough estimate.
      
      for (const auto& chunk : chunk_vec)
      {
        output += get_gotorc_str(chunk.pos.r, chunk.pos.c);
        
        for (const auto& [ch, fg, bg] : chunk.text)
        {
          output += get_color_string(fg, bg);
          if constexpr (std::is_same_v<CharT, char>)
            output.push_back(ch);
          else if constexpr (std::is_same_v<CharT, char32_t>)
            output += utf8::encode_char32_utf8(ch);
          else
            static_assert(std::is_same_v<CharT, char> || std::is_same_v<CharT, char32_t>,
                  "ERROR in Text::print_complex_chunks(): unsupported CharT!");
        }
      }
      
      output += "\033[0m";
      ::term::emit_text(m_term_mode, output);
    }

    
    void print_reset() const
    {
#ifdef _WIN32
      if (!::term::use_ansi_colors(m_term_mode))
      {
        set_color_win_cmd(Color16::White, Color16::Black);
        return;
      }
#endif
      ::term::emit_text(m_term_mode, "\033[0m");
    }
  };
  
}
