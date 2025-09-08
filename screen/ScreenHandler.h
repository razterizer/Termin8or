#pragma once
#include "Text.h"
#include "Styles.h"
#include "../geom/Rectangle.h"
#include "../geom/RC.h"
#include "../drawing/Texture.h"
#include <Core/StringHelper.h>
#include <Core/StringBox.h>
#include <Core/StlUtils.h>
#include <Core/Math.h>
#include <array>
#include <map>
#include <memory>


namespace t8
{
  
  template<int NR, int NC>
  class ScreenHandler;
  
  namespace screen_scaling
  {
    template<int NRo, int NCo, int NRi, int NCi>
    void resample(const ScreenHandler<NRi, NCi>& sh_src,
                  ScreenHandler<NRo, NCo>& sh_dst);
  }
  
  struct OrderedText
  {
    std::string str;
    int r = -1;
    int c = -1;
    int priority = 0;
    Style style;
  };
  
  struct OffscreenBuffer
  {
    Texture* buffer_texture = nullptr;
    RC buffer_screen_pos { 0, 0 }; //
    std::vector<char> exclude_src_chars;
    std::vector<Color> exclude_src_fg_colors;
    std::vector<Color> exclude_src_bg_colors;
    std::vector<Color> dst_fill_bg_colors;
    std::vector<std::pair<Color, Color>> replace_src_dst_bg_colors;
  };
  
  // //////////////////////////////////////////////////
  
  template<int NR = 30, int NC = 80>
  class ScreenHandler
  {
    std::unique_ptr<Text> m_text;
    
    // Draw from top to bottom.
    std::array<std::array<char, NC>, NR> screen_buffer;
    std::array<std::array<Color, NC>, NR> fg_color_buffer;
    std::array<std::array<Color, NC>, NR> bg_color_buffer;
    
    std::string color2str(Color col) const
    {
      switch (col)
      {
        case Color::Transparent:  return "---:";
        case Color::Transparent2: return "===:";
        case Color::Default:      return "Def:";
        case Color::Black:        return "Blk:";
        case Color::DarkRed:      return "DRd:";
        case Color::DarkGreen:    return "DGn:";
        case Color::DarkYellow:   return "DYw:";
        case Color::DarkBlue:     return "DBu:";
        case Color::DarkMagenta:  return "DMa:";
        case Color::DarkCyan:     return "DCn:";
        case Color::LightGray:    return "LGy:";
        case Color::DarkGray:     return "DGy:";
        case Color::Red:          return "Red:";
        case Color::Green:        return "Grn:";
        case Color::Yellow:       return "Ylw:";
        case Color::Blue:         return "Blu:";
        case Color::Magenta:      return "Mga:";
        case Color::Cyan:         return "Cyn:";
        case Color::White:        return "Wht:";
        default: break;
      }
      return "";
    }
    
    std::vector<OrderedText> ordered_texts;
    
  public:
    ScreenHandler()
      : m_text(std::make_unique<Text>())
    {}
    
    void clear()
    {
      for (auto& row : screen_buffer)
        for (auto& col : row)
          col = ' ';
      for (auto& row : fg_color_buffer)
        for (auto& col : row)
          col = Color::Default;
      for (auto& row : bg_color_buffer)
        for (auto& col : row)
          col = Color::Transparent;
    }
    
    bool test_empty(int r, int c) const
    {
      return screen_buffer[r][c] == ' ';
    }
    
    // write_buffer using StringBox.
    void write_buffer(const str::StringBox& sb, int r, int c, Color fg_color, Color bg_color = Color::Transparent)
    {
      auto Nr = static_cast<int>(sb.text_lines.size());
      for (int r_idx = 0; r_idx < Nr; ++r_idx)
        write_buffer(sb.text_lines[r_idx], r + r_idx, c, fg_color, bg_color);
    }
    
    void add_ordered_text(const OrderedText& text)
    {
      ordered_texts.emplace_back(text);
    }
    
    void write_buffer_ordered()
    {
      stlutils::sort(ordered_texts, [](const auto& tA, const auto& tB) { return tA.priority > tB.priority; });
      for (const auto& text : ordered_texts)
        write_buffer(text.str, text.r, text.c, text.style.fg_color, text.style.bg_color);
      // Purge the text vector.
      ordered_texts.clear();
    }
    
    void write_buffer(const std::string& str, const RC& pos, const Style& style)
    {
      write_buffer(str, pos.r, pos.c, style.fg_color, style.bg_color);
    }
    
    void write_buffer(const std::string& str, const RC& pos, Color fg_color, Color bg_color = Color::Transparent)
    {
      write_buffer(str, pos.r, pos.c, fg_color, bg_color);
    }
    
    void write_buffer(const std::string& str, int r, int c, const Style& style)
    {
      write_buffer(str, r, c, style.fg_color, style.bg_color);
    }
    
    void write_buffer(const std::string& str, int r, int c, Color fg_color, Color bg_color = Color::Transparent)
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
                && scr_bg == Color::Transparent)
              {
                scr_ch = str[ci];
                scr_fg = fg_color;
                scr_bg = bg_color;
              }
              else if (scr_bg == Color::Transparent2)
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
    std::string write_buffer_ret(const std::string& str, int r, int c, Color fg_color, Color bg_color = Color::Transparent)
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
              && scr_bg == Color::Transparent)
            {
              scr_ch = str[ci];
              scr_fg = fg_color;
              scr_bg = bg_color;
            }
            else if (c_tot >= 0 && c_tot < NC
                  && scr_bg == Color::Transparent2)
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
    
    void replace_bg_color(Color from_bg_color, Color to_bg_color, Rectangle box)
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
    
    void replace_bg_color(Color to_bg_color, Rectangle box)
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
    
    void replace_bg_color(Color to_bg_color)
    {
      for (int r = 0; r < NR; ++r)
      {
        for (int c = 0; c < NC; ++c)
        {
          bg_color_buffer[r][c] = to_bg_color;
        }
      }
    }
    
    void replace_fg_color(Color to_fg_color, Rectangle box)
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
    
    void replace_fg_color(Color to_fg_color)
    {
      for (int r = 0; r < NR; ++r)
      {
        for (int c = 0; c < NC; ++c)
        {
          fg_color_buffer[r][c] = to_fg_color;
        }
      }
    }
    
    void print_screen_buffer(Color bg_color) const
    {
      std::vector<std::tuple<char, Color, Color>> colored_str;
      colored_str.resize(NR*(NC + 1));
      int i = 0;
      for (int r = 0; r < NR; ++r)
      {
        for (int c = 0; c < NC; ++c)
        {
          Color bg_col_buf = bg_color_buffer[r][c];
          if (bg_col_buf == Color::Transparent || bg_col_buf == Color::Transparent2)
            bg_col_buf = bg_color;
          colored_str[i++] = { screen_buffer[r][c], fg_color_buffer[r][c], bg_col_buf };
        }
        colored_str[i++] = { '\n', Color::Default, Color::Default };
      }
      m_text->print_complex(colored_str);
    }
    
    void print_screen_buffer(Color bg_color, const OffscreenBuffer& offscreen_buffer)
    {
      auto* texture = offscreen_buffer.buffer_texture;
      if (texture == nullptr)
        return;
      
      auto pos = offscreen_buffer.buffer_screen_pos;
      
      for (int r = 0; r < NR; ++r)
      {
        auto rl = r - pos.r;
        for (int c = 0; c < NC; ++c)
        {
          auto cl = c - pos.c;
          
          auto textel = texture->operator()(rl, cl);
          if (!stlutils::contains(offscreen_buffer.dst_fill_bg_colors, textel.bg_color))
            continue;
          
          textel.ch = screen_buffer[r][c];
          textel.fg_color = fg_color_buffer[r][c];
          textel.bg_color = bg_color_buffer[r][c];
          
          if (stlutils::contains(offscreen_buffer.exclude_src_chars, textel.ch))
            continue;
          if (stlutils::contains(offscreen_buffer.exclude_src_fg_colors, textel.fg_color))
            continue;
          if (stlutils::contains(offscreen_buffer.exclude_src_bg_colors, textel.bg_color))
            continue;
          
          for (const auto& rp : offscreen_buffer.replace_src_dst_bg_colors)
            if (textel.bg_color == rp.first)
              textel.bg_color = rp.second;
          
          texture->set_textel(rl, cl, textel);
        }
      }
    }
    
    std::vector<std::string> get_screen_buffer_chars() const
    {
      std::vector<std::string> ret(NR);
      for (int r = 0; r < NR; ++r)
      {
        auto& line = ret[r];
        line.resize(NC);
        for (int c = 0; c < NC; ++c)
          line[c] = screen_buffer[r][c];
      }
      return ret;
    }
    
    Texture export_screen_buffers() const
    {
      Texture texture(NR, NC);
      for (int r = 0; r < NR; ++r)
      {
        for (int c = 0; c < NC; ++c)
        {
          texture.set_textel_char(r, c, screen_buffer[r][c]);
          texture.set_textel_fg_color(r, c, fg_color_buffer[r][c]);
          texture.set_textel_bg_color(r, c, bg_color_buffer[r][c]);
        }
      }
      return texture;
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
    constexpr RC size() const { return { NR, NC }; }
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
    
    void overwrite_data(const std::array<std::array<char, NC>, NR>& new_screen_buffer,
                        const std::array<std::array<Color, NC>, NR>& new_fg_color_buffer,
                        const std::array<std::array<Color, NC>, NR>& new_bg_color_buffer)
    {
      screen_buffer = new_screen_buffer;
      fg_color_buffer = new_fg_color_buffer;
      bg_color_buffer = new_bg_color_buffer;
    }
    
    template<int NRo, int NCo, int NRi, int NCi>
    friend void screen_scaling::resample(const ScreenHandler<NRi, NCi>& sh_src,
                                         ScreenHandler<NRo, NCo>& sh_dst);
  };
  
}
