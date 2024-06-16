#pragma once
#include <Core/Math.h>
#include "Styles.h"

// Bresenham Algorithm.
namespace bresenham
{
  template<int NR, int NC>
  void plot_line_low(SpriteHandler<NR, NC>& sh, float x0, float y0, float x1, float y1,
                     const std::string& str, Text::Color fg_color, Text::Color bg_color)
  {
    auto dx = x1 - x0;
    auto dy = y1 - y0;
    auto yi = 1.f;
    if (dy < 0.f)
    {
      yi = -1.f;
      dy = -dy;
    }
    auto D = 2*dy - dx;
    auto y = y0;
    
    for (auto x : math::linspace(x0, 1.f, x1))
    {
      sh.write_buffer(str, y, x, fg_color, bg_color);
      if (D > 0)
      {
        y += yi;
        D += 2*(dy - dx);
      }
      else
        D += 2*dy;
    }
  }

  template<int NR, int NC>
  void plot_line_high(SpriteHandler<NR, NC>& sh, float x0, float y0, float x1, float y1,
                      const std::string& str, Text::Color fg_color, Text::Color bg_color)
  {
    auto dx = x1 - x0;
    auto dy = y1 - y0;
    auto xi = 1.f;
    if (dx < 0.f)
    {
      xi = -1.f;
      dx = -dx;
    }
    auto D = 2*dx - dy;
    auto x = x0;
    
    for (auto y : math::linspace(y0, 1.f, y1))
    {
      sh.write_buffer(str, y, x, fg_color, bg_color);
      if (D > 0)
      {
        x += xi;
        D += 2*(dx - dy);
      }
      else
        D += 2*dx;
    }
  }

  template<int NR, int NC>
  void plot_line(SpriteHandler<NR, NC>& sh, float x0, float y0, float x1, float y1,
                 const std::string& str, Text::Color fg_color, Text::Color bg_color)
  {
    if (std::abs(y1 - y0) < std::abs(x1 - x0))
    {
      if (x0 > x1)
        plot_line_low(sh, x1, y1, x0, y0, str, fg_color, bg_color);
      else
        plot_line_low(sh, x0, y0, x1, y1, str, fg_color, bg_color);
    }
    else
    {
      if (y0 > y1)
        plot_line_high(sh, x1, y1, x0, y0, str, fg_color, bg_color);
      else
        plot_line_high(sh, x0, y0, x1, y1, str, fg_color, bg_color);
    }
  }
}
 
namespace drawing
{

  enum class OutlineType { Line, Masonry, Masonry2, Masonry3, Masonry4, Temple, Hash, NUM_ITEMS };
  enum class Direction { None, S, SE, E, NE, N, NW, W, SW, NUM_ITEMS };
  template<int NR, int NC>
  void draw_box(SpriteHandler<NR, NC>& sh,
                int r, int c, int len_r, int len_c,
                OutlineType outline_type,
                const styles::Style& outline_style = { Text::Color::Default, Text::Color::Transparent2 },
                const styles::Style& fill_style = { Text::Color::Default, Text::Color::Transparent2 },
                char fill_char = ' ',
                Direction shadow_type = Direction::None,
                const styles::Style& shadow_style = { Text::Color::Default, Text::Color::Transparent2 },
                char shadow_char = ' ')
  {
    // len_r = 3, len_c = 2
    // ###
    // # #
    // # #
    // ###
    //
    // len_r = 4, len_c = 4 (smallest room size by default)
    // #####
    // #   #
    // #   #
    // #   #
    // #####
    
    char outline_n = '#';
    char outline_s = '#';
    std::string outline_w = "#";
    std::string outline_e = "#";
    std::string outline_se = "#";
    std::string outline_ne = "#";
    std::string outline_nw = "#";
    std::string outline_sw = "#";
    switch (outline_type)
    {
      case OutlineType::Line:
        outline_n = '-';
        outline_s = '-';
        outline_w = '|';
        outline_e = '|';
        outline_se = '+';
        outline_ne = '+';
        outline_nw = '+';
        outline_sw = '+';
        break;
      case OutlineType::Masonry:
        outline_n = '=';
        outline_s = '=';
        outline_w = 'H';
        outline_e = 'H';
        outline_se = '#';
        outline_ne = '#';
        outline_nw = '#';
        outline_sw = '#';
        break;
      case OutlineType::Masonry2:
        outline_n = '=';
        outline_s = '=';
        outline_w = '=';
        outline_e = '=';
        outline_se = '=';
        outline_ne = '=';
        outline_nw = '=';
        outline_sw = '=';
        break;
      case OutlineType::Masonry3:
        outline_n = 'M';
        outline_s = 'W';
        outline_w = 'H';
        outline_e = 'H';
        outline_se = '@';
        outline_ne = '@';
        outline_nw = '@';
        outline_sw = '@';
        break;
      case OutlineType::Masonry4:
        outline_n = 'H';
        outline_s = 'H';
        outline_w = 'H';
        outline_e = 'H';
        outline_se = 'H';
        outline_ne = 'H';
        outline_nw = 'H';
        outline_sw = 'H';
        break;
      case OutlineType::Temple:
        outline_n = 'I';
        outline_s = 'I';
        outline_w = 'H';
        outline_e = 'H';
        outline_se = 'O';
        outline_ne = 'O';
        outline_nw = 'O';
        outline_sw = 'O';
        break;
      case OutlineType::Hash:
      default:
        break;
    }
    
    int num_horiz = len_c + 1;
    int num_horiz_inset = num_horiz - 2;
    auto str_horiz_n = outline_nw + str::rep_char(outline_n, num_horiz_inset) + (len_c >= 1 ? outline_ne : "");
    auto str_horiz_s = outline_sw + str::rep_char(outline_s, num_horiz_inset) + (len_c >= 1 ? outline_se : "");
    auto str_fill = str::rep_char(fill_char, num_horiz_inset);
    auto str_shadow_ns = str::rep_char(shadow_char, num_horiz_inset);
    auto str_shadow_ew = std::string(1, shadow_char);
    
    sh.write_buffer(str_horiz_n, r, c, outline_style);
    
    if (len_r >= 2)
    {
      if (shadow_type == Direction::NW || shadow_type == Direction::N || shadow_type == Direction::NE)
        sh.write_buffer(str_shadow_ns, r + 1, c + 1, shadow_style);
      else if (shadow_type == Direction::SW || shadow_type == Direction::S || shadow_type == Direction::SE)
        sh.write_buffer(str_shadow_ns, r + len_r - 1, c + 1, shadow_style);
    }
    
    bool has_west_shadow = len_c >= 2 && (shadow_type == Direction::SW || shadow_type == Direction::W || shadow_type == Direction::NW);
    bool has_east_shadow = len_c >= 2 && (shadow_type == Direction::SE || shadow_type == Direction::E || shadow_type == Direction::NE);
      
    for (int i = r + 1; i <= r + len_r - 1; ++i)
    {
      if (has_west_shadow)
        sh.write_buffer(str_shadow_ew, i, c + 1, shadow_style);
      else if (has_east_shadow)
        sh.write_buffer(str_shadow_ew, i, c + len_c - 1, shadow_style);
        
      sh.write_buffer(outline_w, i, c, outline_style);
      sh.write_buffer(str_fill, i, c + 1, fill_style);
      sh.write_buffer(outline_e, i, c + len_c, outline_style);
    }
    sh.write_buffer(str_horiz_s, r + len_r, c, outline_style);
  }
}
