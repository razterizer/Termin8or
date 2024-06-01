#pragma once
#include "Text.h"
#include "Rectangle.h"
#include <Core/StringHelper.h>
#include <Core/TextBox.h>
#include <Core/StlUtils.h>
#include <array>

struct OrderedText
{
  std::string str;
  int r = -1;
  int c = -1;
  int priority = 0;
  Text::Color fg_color = Text::Color::Default;
  Text::Color bg_color = Text::Color::Transparent;
};


template<int NR = 30, int NC = 80>
class SpriteHandler
{
  // Draw from top to bottom.
  std::array<std::array<char, NC>, NR> screen_buffer;
  std::array<std::array<Text::Color, NC>, NR> fg_color_buffer;
  std::array<std::array<Text::Color, NC>, NR> bg_color_buffer;

  std::string color2str(Text::Color col) const
  {
    switch (col)
    {
      case Text::Color::Transparent:  return "---:";
      case Text::Color::Transparent2: return "===:";
      case Text::Color::Default:      return "Def:";
      case Text::Color::Black:        return "Blk:";
      case Text::Color::DarkRed:      return "DRd:";
      case Text::Color::DarkGreen:    return "DGn:";
      case Text::Color::DarkYellow:   return "DYw:";
      case Text::Color::DarkBlue:     return "DBu:";
      case Text::Color::DarkMagenta:  return "DMa:";
      case Text::Color::DarkCyan:     return "DCn:";
      case Text::Color::LightGray:    return "LGy:";
      case Text::Color::DarkGray:     return "DGy:";
      case Text::Color::Red:          return "Red:";
      case Text::Color::Green:        return "Grn:";
      case Text::Color::Yellow:       return "Ylw:";
      case Text::Color::Blue:         return "Blu:";
      case Text::Color::Magenta:      return "Mga:";
      case Text::Color::Cyan:         return "Cyn:";
      case Text::Color::White:        return "Wht:";
      default: break;
    }
    return "";
  }

  std::vector<OrderedText> ordered_texts;

public:
  void clear()
  {
    for (auto& row : screen_buffer)
      for (auto& col : row)
        col = ' ';
    for (auto& row : fg_color_buffer)
      for (auto& col : row)
        col = Text::Color::Default;
    for (auto& row : bg_color_buffer)
      for (auto& col : row)
        col = Text::Color::Transparent;
  }

  bool test_empty(int r, int c) const
  {
    return screen_buffer[r][c] == ' ';
  }
  
  // write_buffer using TextBox.
  void write_buffer(const TextBox& tb, int r, int c, Text::Color fg_color, Text::Color bg_color = Text::Color::Transparent)
  {
    auto Nr = static_cast<int>(tb.text_lines.size());
    for (int r_idx = 0; r_idx < Nr; ++r_idx)
      write_buffer(tb.text_lines[r_idx], r + r_idx, c, fg_color, bg_color);
  }
  
  void add_ordered_text(const OrderedText& text)
  {
    ordered_texts.emplace_back(text);
  }
  
  void write_buffer_ordered()
  {
    stlutils::sort(ordered_texts, [](const auto& tA, const auto& tB) { return tA.priority > tB.priority; });
    for (const auto& text : ordered_texts)
      write_buffer(text.str, text.r, text.c, text.fg_color, text.bg_color);
    // Purge the text vector.
    ordered_texts.clear();
  }

  void write_buffer(const std::string& str, int r, int c, Text::Color fg_color, Text::Color bg_color = Text::Color::Transparent)
  {
    if (str.empty())
      return;
    //if (c >= 0 && str.size() + c <= NC)
    {
      if (r >= 0 && r < NR)
      {
        int n = static_cast<int>(str.size());
        for (int ci = 0; ci < n; ++ci)
        {
          int c_tot = c + ci;
          if (c_tot >= 0 && c_tot < NC)
          {
            auto& scr_ch = screen_buffer[r][c_tot];
            auto& scr_fg = fg_color_buffer[r][c_tot];
            auto& scr_bg = bg_color_buffer[r][c_tot];
            if (scr_ch == ' '
              && scr_bg == Text::Color::Transparent)
            {
              scr_ch = str[ci];
              scr_fg = fg_color;
              scr_bg = bg_color;
            }
            else if (scr_bg == Text::Color::Transparent2)
            {
              scr_bg = bg_color;
              if (scr_ch == ' ')
              {
                scr_ch = str[ci];
                scr_fg = fg_color;
              }
            }
          }
        }
      }
    }
  }
  
  // Return copy of str but with spaces where tests failed.
  std::string write_buffer_ret(const std::string& str, int r, int c, Text::Color fg_color, Text::Color bg_color = Text::Color::Transparent)
  {
    if (str.empty())
      return "";
    std::string str_out = str;
    //if (c >= 0 && str.size() + c <= NC)
    {
      if (r >= 0 && r < NR)
      {
        int n = static_cast<int>(str.size());
        for (int ci = 0; ci < n; ++ci)
        {
          int c_tot = c + ci;
          auto& scr_ch = screen_buffer[r][c_tot];
          auto& scr_fg = fg_color_buffer[r][c_tot];
          auto& scr_bg = bg_color_buffer[r][c_tot];
          if (c_tot >= 0 && c_tot < NC
              && scr_ch == ' '
              && scr_bg == Text::Color::Transparent)
          {
            scr_ch = str[ci];
            scr_fg = fg_color;
            scr_bg = bg_color;
          }
          else if (c_tot >= 0 && c_tot < NC
                   && scr_bg == Text::Color::Transparent2)
          {
            scr_bg = bg_color;
            if (scr_ch == ' ')
            {
              scr_ch = str[ci];
              scr_fg = fg_color;
            }
          }
          else
          {
            str_out[ci] = ' ';
          }
        }
      }
    }
    return str_out;
  }

  void replace_bg_color(Text::Color from_bg_color, Text::Color to_bg_color, ttl::Rectangle box)
  {
    for (int r = 0; r < NR; ++r)
    {
      for (int c = 0; c < NC; ++c)
      {
        if (box.is_inside(r, c))
        {
          if (bg_color_buffer[r][c] == from_bg_color)
            bg_color_buffer[r][c] = to_bg_color;
        }
      }
    }
  }
  
  void replace_bg_color(Text::Color to_bg_color, ttl::Rectangle box)
  {
    for (int r = 0; r < NR; ++r)
    {
      for (int c = 0; c < NC; ++c)
      {
        if (box.is_inside(r, c))
        {
          bg_color_buffer[r][c] = to_bg_color;
        }
      }
    }
  }
  
  void replace_fg_color(Text::Color to_fg_color, ttl::Rectangle box)
  {
    for (int r = 0; r < NR; ++r)
    {
      for (int c = 0; c < NC; ++c)
      {
        if (box.is_inside(r, c))
        {
          fg_color_buffer[r][c] = to_fg_color;
        }
      }
    }
  }

  void print_screen_buffer(Text& t, Text::Color bg_color) const
  {
    std::vector<std::tuple<char, Text::Color, Text::Color>> colored_str;
    colored_str.resize(NR*(NC + 1));
    int i = 0;
    for (int r = 0; r < NR; ++r)
    {
      for (int c = 0; c < NC; ++c)
      {
        Text::Color bg_col_buf = bg_color_buffer[r][c];
        if (bg_col_buf == Text::Color::Transparent || bg_col_buf == Text::Color::Transparent2)
          bg_col_buf = bg_color;
        colored_str[i++] = { screen_buffer[r][c], fg_color_buffer[r][c], bg_col_buf };
      }
      colored_str[i++] = { '\n', Text::Color::Default, Text::Color::Default };
    }
    t.print_complex(colored_str);
  }

  void print_screen_buffer_chars() const
  {
    for (int r = 0; r < NR; ++r)
    {
      for (int c = 0; c < NC; ++c)
        printf("%c", screen_buffer[r][c]);
      printf("\n");
    }
  }

  void print_screen_buffer_fg_colors() const
  {
    for (int r = 0; r < NR; ++r)
    {
      for (int c = 0; c < NC; ++c)
        printf("%s", color2str(fg_color_buffer[r][c]).c_str());
      printf("\n");
    }
  }

  void print_screen_buffer_bg_colors() const
  {
    for (int r = 0; r < NR; ++r)
    {
      for (int c = 0; c < NC; ++c)
        printf("%s", color2str(bg_color_buffer[r][c]).c_str());
      printf("\n");
    }
  }
  
  constexpr int num_rows() const { return NR; }
  constexpr int num_cols() const { return NC; }
  constexpr int num_rows_inset() const
  {
    auto nri = static_cast<int>(NR) - 2;
    return nri < 0 ? 0 : nri;
  }
  constexpr int num_cols_inset() const
  {
    auto nci = static_cast<int>(NC) - 2;
    return nci < 0 ? 0 : nci;
  }
};

