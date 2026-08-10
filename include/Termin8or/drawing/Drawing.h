#pragma once
#include <Core/MathUtils.h>
#include <Core/bool_vector.h>
#include "../screen/Styles.h"
#include "../screen/Glyph.h"
#include "../screen/GlyphString.h"
#include "../drawing/Texture.h"

namespace t8
{
  template<int NR, int NC, typename CharT>
  class ScreenHandler;
}

namespace t8x
{
  using RC = t8::RC;
  using Rectangle = t8::Rectangle;
  using Color = t8::Color;
  using Color16 = t8::Color16;
  using Style = t8::Style;
  using ShadeType = t8::ShadeType;
  using Texture = t8::Texture;
  template<int NR, int NC, typename CharT>
  using ScreenHandler = t8::ScreenHandler<NR, NC, CharT>;
  
  
  // Bresenham Algorithm.
  void plot_line_low(float r0, float c0, float r1, float c1, std::vector<RC>& points)
  {
    auto dr = r1 - r0;
    auto dc = c1 - c0;
    auto ri = 1.f;
    if (dr < 0.f)
    {
      ri = -1.f;
      dr = -dr;
    }
    auto D = 2*dr - dc;
    auto r = r0;
    
    for (auto c : math::linspace(c0, 1.f, c1))
    {
      points.emplace_back(math::roundI(r), math::roundI(c));
      if (D > 0)
      {
        r += ri;
        D += 2*(dr - dc);
      }
      else
        D += 2*dr;
    }
  }

  void plot_line_high(float r0, float c0, float r1, float c1, std::vector<RC>& points)
  {
    auto dr = r1 - r0;
    auto dc = c1 - c0;
    auto ci = 1.f;
    if (dc < 0.f)
    {
      ci = -1.f;
      dc = -dc;
    }
    auto D = 2*dc - dr;
    auto c = c0;
    
    for (auto r : math::linspace(r0, 1.f, r1))
    {
      points.emplace_back(math::roundI(r), math::roundI(c));
      if (D > 0)
      {
        c += ci;
        D += 2*(dc - dr);
      }
      else
        D += 2*dc;
    }
  }
  
  void plot_line(float r0, float c0, float r1, float c1, std::vector<RC>& points)
  {
    if (std::abs(r1 - r0) < std::abs(c1 - c0))
    {
      if (c0 > c1)
        plot_line_low(r1, c1, r0, c0, points);
      else
        plot_line_low(r0, c0, r1, c1, points);
    }
    else
    {
      if (r0 > r1)
        plot_line_high(r1, c1, r0, c0, points);
      else
        plot_line_high(r0, c0, r1, c1, points);
    }
  }
  
  void plot_line(const RC& p0, const RC& p1, std::vector<RC>& points)
  {
    plot_line(
      static_cast<float>(p0.r),
      static_cast<float>(p0.c), 
      static_cast<float>(p1.r), 
      static_cast<float>(p1.c), 
      points);
  }

  template<int NR, int NC, typename CharT>
  void plot_line(ScreenHandler<NR, NC, CharT>& sh, float r0, float c0, float r1, float c1,
                 const t8::GlyphString& gstr, Color fg_color, Color bg_color)
  {
    std::vector<RC> points;
    plot_line(r0, c0, r1, c1, points);
    for (const auto& pt : points)
      sh.write_buffer(gstr, pt.r, pt.c, fg_color, bg_color);
  }
  
  template<int NR, int NC, typename CharT>
  void plot_line(ScreenHandler<NR, NC, CharT>& sh, const RC& p0, const RC& p1,
                 const t8::GlyphString& gstr, Color fg_color, Color bg_color)
  {
    std::vector<RC> points;
    plot_line(p0, p1, points);
    for (const auto& pt : points)
      sh.write_buffer(gstr, pt.r, pt.c, fg_color, bg_color);
  }
  
  // ////////////////////////////////

  enum class OutlineType
  {
    Line, Hash,
    Masonry, Masonry2, Masonry3, Masonry4, Temple, // Rogue-like styles.
    Unicode_SingleLine, Unicode_SingleLineRounded,
    Unicode_SingleLineThick, Unicode_SingleLineThicker,
    Unicode_DoubleLine,
    Unicode_BlockDark, Unicode_BlockNormal, Unicode_BlockLight,
    Unicode_Masonry, Unicode_Checker, Unicode_Temple, // Rogue-like styles.
    NUM_ITEMS
  };
  enum class SolarDirection
  {
    Nadir, Zenith,
    N, NE, E, SE, S, SW, W, NW,
    N_Low, NE_Low, E_Low, SE_Low, S_Low, SW_Low, W_Low, NW_Low,
    NUM_ITEMS
  };
  
  template<int NR, int NC, typename CharT>
  void draw_box_outline(ScreenHandler<NR, NC, CharT>& sh,
                        int r, int c, int len_r, int len_c,
                        OutlineType outline_type,
                        const Style& outline_style = { Color16::Default, Color16::Transparent2 },
                        const bool_vector& light_field = {})
  {
    // len_r = 4, len_c = 3
    // ###
    // # #
    // # #
    // ###
    
    using namespace t8::literals;
    
    t8::GlyphString outline_n = "#"_gs;
    t8::GlyphString outline_s = "#"_gs;
    t8::Glyph outline_w = '#';
    t8::Glyph outline_e = '#';
    t8::Glyph outline_se = '#';
    t8::Glyph outline_ne = '#';
    t8::Glyph outline_nw = '#';
    t8::Glyph outline_sw = '#';
    t8::GlyphString outline_n2;
    t8::GlyphString outline_s2;
    t8::Glyph outline_w2;
    t8::Glyph outline_e2;
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
        break;
      case OutlineType::Unicode_SingleLine:
        outline_n = t8::Glyph { 0x2500, '-' };
        outline_s = t8::Glyph { 0x2500, '-' };
        outline_w = t8::Glyph { 0x2502, '|' };
        outline_e = t8::Glyph { 0x2502, '|' };
        outline_se = t8::Glyph { 0x2518, '+' };
        outline_ne = t8::Glyph { 0x2510, '+' };
        outline_nw = t8::Glyph { 0x250C, '+' };
        outline_sw = t8::Glyph { 0x2514, '+' };
        break;
      case OutlineType::Unicode_SingleLineRounded:
        outline_n = t8::Glyph { 0x2500, '-' };
        outline_s = t8::Glyph { 0x2500, '-' };
        outline_w = t8::Glyph { 0x2502, '|' };
        outline_e = t8::Glyph { 0x2502, '|' };
        outline_se = t8::Glyph { 0x256F, '+' };
        outline_ne = t8::Glyph { 0x256E, '+' };
        outline_nw = t8::Glyph { 0x256D, '+' };
        outline_sw = t8::Glyph { 0x2570, '+' };
        break;
      case OutlineType::Unicode_SingleLineThick:
        outline_n = t8::Glyph { 0x2501, '-' };
        outline_s = t8::Glyph { 0x2501, '-' };
        outline_w = t8::Glyph { 0x2503, '|' };
        outline_e = t8::Glyph { 0x2503, '|' };
        outline_se = t8::Glyph { 0x251B, '+' };
        outline_ne = t8::Glyph { 0x2513, '+' };
        outline_nw = t8::Glyph { 0x250F, '+' };
        outline_sw = t8::Glyph { 0x2517, '+' };
        break;
      case OutlineType::Unicode_SingleLineThicker:
        outline_n = t8::Glyph { 0x2580, '-' };
        outline_s = t8::Glyph { 0x2584, '-' };
        outline_w = t8::Glyph { 0x258C, '|' };
        outline_e = t8::Glyph { 0x2590, '|' };
        outline_se = t8::Glyph { 0x259F, '+' };
        outline_ne = t8::Glyph { 0x259C, '+' };
        outline_nw = t8::Glyph { 0x259B, '+' };
        outline_sw = t8::Glyph { 0x2599, '+' };
        break;
      case OutlineType::Unicode_DoubleLine:
        outline_n = t8::Glyph { 0x2550, '=' };
        outline_s = t8::Glyph { 0x2550, '=' };
        outline_w = t8::Glyph { 0x2551, '|' };
        outline_e = t8::Glyph { 0x2551, '|' };
        outline_se = t8::Glyph { 0x255D, '+' };
        outline_ne = t8::Glyph { 0x2557, '+' };
        outline_nw = t8::Glyph { 0x2554, '+' };
        outline_sw = t8::Glyph { 0x255A, '+' };
        break;
      case OutlineType::Unicode_BlockDark:
        outline_n = t8::Glyph { 0x2593, '#' };
        outline_s = t8::Glyph { 0x2593, '#' };
        outline_w = t8::Glyph { 0x2593, '#' };
        outline_e = t8::Glyph { 0x2593, '#' };
        outline_se = t8::Glyph { 0x2593, '#' };
        outline_ne = t8::Glyph { 0x2593, '#' };
        outline_nw = t8::Glyph { 0x2593, '#' };
        outline_sw = t8::Glyph { 0x2593, '#' };
        break;
      case OutlineType::Unicode_BlockNormal:
        outline_n = t8::Glyph { 0x2592, '#' };
        outline_s = t8::Glyph { 0x2592, '#' };
        outline_w = t8::Glyph { 0x2592, '#' };
        outline_e = t8::Glyph { 0x2592, '#' };
        outline_se = t8::Glyph { 0x2592, '#' };
        outline_ne = t8::Glyph { 0x2592, '#' };
        outline_nw = t8::Glyph { 0x2592, '#' };
        outline_sw = t8::Glyph { 0x2592, '#' };
        break;
      case OutlineType::Unicode_BlockLight:
        outline_n = t8::Glyph { 0x2591, '#' };
        outline_s = t8::Glyph { 0x2591, '#' };
        outline_w = t8::Glyph { 0x2591, '#' };
        outline_e = t8::Glyph { 0x2591, '#' };
        outline_se = t8::Glyph { 0x2591, '#' };
        outline_ne = t8::Glyph { 0x2591, '#' };
        outline_nw = t8::Glyph { 0x2591, '#' };
        outline_sw = t8::Glyph { 0x2591, '#' };
        break;
      case OutlineType::Unicode_Masonry:
        outline_n = t8::Glyph { 0x252C, '=' };
        outline_s = t8::Glyph { 0x2534, '=' };
        outline_w = t8::Glyph { 0x251C, 'H' };
        outline_e = t8::Glyph { 0x2524, 'H' };
        outline_se = t8::Glyph { 0x253C, '#' };
        outline_ne = t8::Glyph { 0x253C, '#' };
        outline_nw = t8::Glyph { 0x253C, '#' };
        outline_sw = t8::Glyph { 0x253C, '#' };
        outline_n2 = t8::Glyph { 0x2534, '=' };
        outline_s2 = t8::Glyph { 0x252C, '=' };
        outline_w2 = t8::Glyph { 0x2524, 'H' };
        outline_e2 = t8::Glyph { 0x251C, 'H' };
        break;
      case OutlineType::Unicode_Checker:
        outline_n = t8::Glyph { 0x2580, 'M' };
        outline_s = t8::Glyph { 0x2584, 'W' };
        outline_w = t8::Glyph { 0x258C, 'H' };
        outline_e = t8::Glyph { 0x2590, 'H' };
        outline_se = t8::Glyph { 0x259F, '@' };
        outline_ne = t8::Glyph { 0x259C, '@' };
        outline_nw = t8::Glyph { 0x259B, '@' };
        outline_sw = t8::Glyph { 0x2599, '@' };
        outline_n2 = t8::Glyph { 0x2584, 'M' };
        outline_s2 = t8::Glyph { 0x2580, 'W' };
        outline_w2 = t8::Glyph { 0x2590, 'H' };
        outline_e2 = t8::Glyph { 0x258C, 'H' };
        break;
      case OutlineType::Unicode_Temple:
        outline_n = t8::Glyph { 0x20B8, 'I' };
        outline_s = t8::Glyph { 0x27C2, 'I' };
        outline_w = t8::Glyph { 0x27DD, 'H' };
        outline_e = t8::Glyph { 0x27DE, 'H' };
        outline_se = t8::Glyph { 0x256F, 'O' };
        outline_ne = t8::Glyph { 0x256E, 'O' };
        outline_nw = t8::Glyph { 0x256D, 'O' };
        outline_sw = t8::Glyph { 0x2570, 'O' };
        //outline_n2 = t8::Glyph { 0x2534, 'I' };
        //outline_s2 = t8::Glyph { 0x252C, 'I' };
        //outline_w2 = t8::Glyph { 0x2524, 'H' };
        //outline_e2 = t8::Glyph { 0x251C, 'H' };
        break;
      default:
        break;
    }
    
    auto f_has_light = [&](int r0, int c0)
    {
      if (light_field.empty())
        return false;
      return static_cast<bool>(light_field[r0 * len_c + c0]);
    };
    
    auto f_shade_style = [&](const Style& style, int r0, int c0)
    {
      return shade_style(style, f_has_light(r0, c0) ?
          ShadeType::Bright : ShadeType::Unchanged, true);
    };
    
    // Outline
    int num_horiz = len_c;
    int num_horiz_inset = num_horiz - 2;
    
    t8::GlyphString str_horiz_n_inset;
    if (outline_n2.empty())
      str_horiz_n_inset = str::rep_str(outline_n, num_horiz_inset);
    else
      for (int i = 0; i < num_horiz_inset; ++i)
        str_horiz_n_inset += (i % 2 == 0) ? outline_n : outline_n2;
    
    t8::GlyphString str_horiz_s_inset;
    if (outline_s2.empty())
      str_horiz_s_inset = str::rep_str(outline_s, num_horiz_inset);
    else
      for (int i = 0; i < num_horiz_inset; ++i)
        str_horiz_s_inset += (i % 2 == 0) ? outline_s : outline_s2;
    auto str_horiz_n = outline_nw + str_horiz_n_inset + (len_c >= 2 ? outline_ne : ""_gs);
    auto str_horiz_s = outline_sw + str_horiz_s_inset + (len_c >= 2 ? outline_se : ""_gs);

    for (int j = 0; j < len_c; ++j)
    {
      sh.write_buffer(str_horiz_n[j], r, c + j, f_shade_style(outline_style, 0, j));
    
      sh.write_buffer(str_horiz_s[j], r + len_r - 1, c + j, f_shade_style(outline_style, len_r - 1, j));
    }

    auto f_sel_vert_glyph = [](const t8::Glyph& glyph0, const t8::Glyph& glyph1, int i) -> t8::Glyph
    {
      if (glyph1.empty())
        return glyph0;
      return (i % 2 == 0) ? glyph0 : glyph1;
    };
    
    for (int i = r + 1; i < r + len_r - 1; ++i)
    {
      auto r0 = i - r;
      sh.write_buffer(f_sel_vert_glyph(outline_w, outline_w2, i), i, c, f_shade_style(outline_style, r0, 0));
      sh.write_buffer(f_sel_vert_glyph(outline_e, outline_e2, i), i, c + len_c - 1, f_shade_style(outline_style, r0, len_c - 1));
    }
  }
  
  template<int NR, int NC, typename CharT>
  void draw_box_outline(ScreenHandler<NR, NC, CharT>& sh,
                        const Rectangle& bb,
                        OutlineType outline_type,
                        const Style& outline_style = { Color16::Default, Color16::Transparent2 },
                        const bool_vector& light_field = {})
  {
    draw_box_outline(sh, bb.r, bb.c, bb.r_len, bb.c_len, outline_type, outline_style, light_field);
  }
  
  template<int NR, int NC, typename CharT>
  void draw_box(ScreenHandler<NR, NC, CharT>& sh,
                int r, int c, int len_r, int len_c,
                const Style& fill_style = { Color16::Default, Color16::Transparent2 },
                t8::Glyph fill_glyph = ' ',
                SolarDirection shadow_type = SolarDirection::Zenith,
                const Style& shadow_style = { Color16::Default, Color16::Transparent2 },
                t8::Glyph shadow_glyph = ' ',
                const bool_vector& light_field = {})
  {
  
    auto f_has_light = [&](int r0, int c0)
    {
      if (light_field.empty())
        return false;
      return static_cast<bool>(light_field[r0 * len_c + c0]);
    };
    
    auto f_shade_style = [&](const Style& style, int r0, int c0)
    {
      return shade_style(style, f_has_light(r0, c0) ?
          ShadeType::Bright : ShadeType::Unchanged, true);
    };
    
    // Filling
    auto shadow_ns = shadow_glyph; //str::rep_char(shadow_char, num_horiz_inset);
    auto shadow_ew = shadow_glyph; //str::rep_char(shadow_char, num_horiz_inset);
    
    if (len_r >= 3)
    {
      for (int i = 1; i < len_c - 1; ++i)
      {
        if (shadow_type == SolarDirection::NW || shadow_type == SolarDirection::N || shadow_type == SolarDirection::NE)
          sh.write_buffer(shadow_ns, r + 1, i + c, f_shade_style(shadow_style, 1, i));
        else if (shadow_type == SolarDirection::SW || shadow_type == SolarDirection::S || shadow_type == SolarDirection::SE)
          sh.write_buffer(shadow_ns, r + len_r - 2, i + c, f_shade_style(shadow_style, len_r - 2, i));
        else if (shadow_type == SolarDirection::SW_Low || shadow_type == SolarDirection::S_Low || shadow_type == SolarDirection::SE_Low)
          sh.write_buffer(shadow_ns, r + 1, i + c, f_shade_style(fill_style, 1, i));
        else if (shadow_type == SolarDirection::NW_Low || shadow_type == SolarDirection::N_Low || shadow_type == SolarDirection::NE_Low)
          sh.write_buffer(shadow_ns, r + len_r - 2, i + c, f_shade_style(fill_style, len_r - 2, i));
      }
    }
    
    bool has_west_shadow = len_c >= 3 && (shadow_type == SolarDirection::SW || shadow_type == SolarDirection::W || shadow_type == SolarDirection::NW);
    bool has_east_shadow = len_c >= 3 && (shadow_type == SolarDirection::SE || shadow_type == SolarDirection::E || shadow_type == SolarDirection::NE);
    bool has_west_twilight = len_c >= 3 && (shadow_type == SolarDirection::SE_Low || shadow_type == SolarDirection::E_Low || shadow_type == SolarDirection::NE_Low);
    bool has_east_twilight = len_c >= 3 && (shadow_type == SolarDirection::SW_Low || shadow_type == SolarDirection::W_Low || shadow_type == SolarDirection::NW_Low);
    
    for (int i = r + 1; i < r + len_r - 1; ++i)
    {
      auto r0 = i - r;
      if (has_west_shadow)
        sh.write_buffer(shadow_ew, i, c + 1, f_shade_style(shadow_style, r0, 1));
      else if (has_east_shadow)
        sh.write_buffer(shadow_ew, i, c + len_c - 2, f_shade_style(shadow_style, r0, len_c - 2));
      else if (has_west_twilight)
        sh.write_buffer(shadow_ew, i, c + 1, f_shade_style(fill_style, r0, 1));
      else if (has_east_twilight)
        sh.write_buffer(shadow_ew, i, c + len_c - 2, f_shade_style(fill_style, r0, len_c - 2));
      
      bool night_or_twilight = shadow_type == SolarDirection::Nadir ||
        shadow_type == SolarDirection::N_Low ||
        shadow_type == SolarDirection::NE_Low ||
        shadow_type == SolarDirection::E_Low ||
        shadow_type == SolarDirection::SE_Low ||
        shadow_type == SolarDirection::S_Low ||
        shadow_type == SolarDirection::SW_Low ||
        shadow_type == SolarDirection::W_Low ||
        shadow_type == SolarDirection::NW_Low;
      auto style = night_or_twilight ? shadow_style : fill_style;
      for (int j = 1; j < len_c - 1; ++j)
        sh.write_buffer(fill_glyph, i, j + c, f_shade_style(style, r0, j));
    }
  }
  
  // #NOTE: Walled box. Meant to go with draw_box_outline().
  template<int NR, int NC, typename CharT>
  void draw_box(ScreenHandler<NR, NC, CharT>& sh,
                const Rectangle& bb,
                const Style& fill_style = { Color16::Default, Color16::Transparent2 },
                t8::Glyph fill_glyph = ' ',
                SolarDirection shadow_type = SolarDirection::Zenith,
                const Style& shadow_style = { Color16::Default, Color16::Transparent2 },
                t8::Glyph shadow_glyph = ' ',
                const bool_vector& light_field = {})
  {
    draw_box(sh,
             bb.r, bb.c, bb.r_len, bb.c_len,
             fill_style,
             fill_glyph,
             shadow_type,
             shadow_style,
             shadow_glyph,
             light_field);
  }
  
  // E.g.
  // r = 5, c = 6, len_r = 9, len_c = 7,
  // fill_texture.size.r = 9, fill_texture.size.c = 7,
  // shadow_texture.size.r = 9, shadow_texture.size.c = 6.
  // #NOTE: Walled box. Meant to go with draw_box_outline().
  template<int NR, int NC, typename CharT>
  void draw_box_textured(ScreenHandler<NR, NC, CharT>& sh,
                         int r, int c, int len_r, int len_c,
                         SolarDirection shadow_type = SolarDirection::Zenith,
                         const Texture& fill_texture = {},
                         const Texture& shadow_texture = {},
                         const bool_vector& light_field = {},
                         bool is_underground = false,
                         const RC& tex_offset = { 0, 0 })
  {
    
    auto f_has_light = [&](int r0, int c0)
    {
      if (light_field.empty())
        return false;
      return static_cast<bool>(light_field[r0 * len_c + c0]);
    };
    
    // Filling
    if (len_r >= 3)
    {
      for (int i = 1; i < len_c - 1; ++i)
      {
        if (shadow_type == SolarDirection::NW || shadow_type == SolarDirection::N || shadow_type == SolarDirection::NE)
        {
          const auto tex_pos = tex_offset + RC { 0, i - 1 };
          const auto nt = fill_texture(tex_pos);
          const auto st = shadow_texture(tex_pos);
          const auto& t = f_has_light(1, i) ? nt : st;
          sh.write_buffer(t.glyph, r + 1, i + c, t.get_style());
        }
        else if (shadow_type == SolarDirection::SW || shadow_type == SolarDirection::S || shadow_type == SolarDirection::SE)
        {
          const auto tex_pos = tex_offset + RC { len_r - 3, i - 1 };
          const auto nt = fill_texture(tex_pos);
          const auto st = shadow_texture(tex_pos);
          const auto& t = f_has_light(len_r - 2, i) ? nt : st;
          sh.write_buffer(t.glyph, r + len_r - 2, i + c, t.get_style());
        }
        else if (shadow_type == SolarDirection::SW_Low || shadow_type == SolarDirection::S_Low || shadow_type == SolarDirection::SE_Low)
        {
          const auto tex_pos = tex_offset + RC { 0, i - 1 };
          const auto nt = fill_texture(tex_pos);
          sh.write_buffer(nt.glyph, r + 1, i + c, nt.get_style());
        }
        else if (shadow_type == SolarDirection::NW_Low || shadow_type == SolarDirection::N_Low || shadow_type == SolarDirection::NE_Low)
        {
          const auto tex_pos = tex_offset + RC { len_r - 3, i - 1 };
          const auto nt = fill_texture(tex_pos);
          sh.write_buffer(nt.glyph, r + len_r - 2, i + c, nt.get_style());
        }
      }
    }

    bool has_west_shadow = len_c >= 3 && (shadow_type == SolarDirection::SW || shadow_type == SolarDirection::W || shadow_type == SolarDirection::NW);
    bool has_east_shadow = len_c >= 3 && (shadow_type == SolarDirection::SE || shadow_type == SolarDirection::E || shadow_type == SolarDirection::NE);
    bool has_west_twilight = len_c >= 3 && (shadow_type == SolarDirection::SE_Low || shadow_type == SolarDirection::E_Low || shadow_type == SolarDirection::NE_Low);
    bool has_east_twilight = len_c >= 3 && (shadow_type == SolarDirection::SW_Low || shadow_type == SolarDirection::W_Low || shadow_type == SolarDirection::NW_Low);
    
    for (int i = r + 1; i < r + len_r - 1; ++i)
    {
      auto r0 = i - r;
      if (has_west_shadow)
      {
        const auto tex_pos = tex_offset + RC { r0 - 1, 0 };
        const auto nt = fill_texture(tex_pos);
        const auto st = shadow_texture(tex_pos);
        const auto& t = f_has_light(r0, 1) ? nt : st;
        sh.write_buffer(t.glyph, i, c + 1, t.get_style());
      }
      else if (has_east_shadow)
      {
        const auto tex_pos = tex_offset + RC { r0 - 1, len_c - 3 };
        const auto nt = fill_texture(tex_pos);
        const auto st = shadow_texture(tex_pos);
        const auto& t = f_has_light(r0, len_c - 2) ? nt : st;
        sh.write_buffer(t.glyph, i, c + len_c - 2, t.get_style());
      }
      else if (has_west_twilight)
      {
        const auto tex_pos = tex_offset + RC { r0 - 1, 0 };
        const auto nt = fill_texture(tex_pos);
        sh.write_buffer(nt.glyph, i, c + 1, nt.get_style());
      }
      else if (has_east_twilight)
      {
        const auto tex_pos = tex_offset + RC { r0 - 1, len_c - 3 };
        const auto nt = fill_texture(tex_pos);
        sh.write_buffer(nt.glyph, i, c + len_c - 2, nt.get_style());
      }
      
      bool night_or_twilight = shadow_type == SolarDirection::Nadir ||
        shadow_type == SolarDirection::N_Low ||
        shadow_type == SolarDirection::NE_Low ||
        shadow_type == SolarDirection::E_Low ||
        shadow_type == SolarDirection::SE_Low ||
        shadow_type == SolarDirection::S_Low ||
        shadow_type == SolarDirection::SW_Low ||
        shadow_type == SolarDirection::W_Low ||
        shadow_type == SolarDirection::NW_Low;
      for (int j = 1; j < len_c - 1; ++j)
      {
        const auto tex_pos = tex_offset + RC { r0 - 1, j - 1 };
        const auto nt = fill_texture(tex_pos);
        const auto st = shadow_texture(tex_pos);
        const auto& t = f_has_light(r0, j) ? nt : ((is_underground || night_or_twilight) ? st : nt);
        sh.write_buffer(t.glyph, i, j + c, t.get_style());
      }
    }
  }
  
  // #NOTE: Walled box. Meant to go with draw_box_outline().
  template<int NR, int NC, typename CharT>
  void draw_box_textured(ScreenHandler<NR, NC, CharT>& sh,
                         const Rectangle& bb,
                         SolarDirection shadow_type = SolarDirection::Zenith,
                         const Texture& fill_texture = {},
                         const Texture& shadow_texture = {},
                         const bool_vector& light_field = {},
                         bool is_underground = false,
                         const RC& tex_offset = { 0, 0 })
  {
    draw_box_textured(sh,
                      bb.r, bb.c, bb.r_len, bb.c_len,
                      shadow_type,
                      shadow_texture,
                      light_field,
                      is_underground,
                      tex_offset);
  }
  
  // #NOTE: "Unwalled box".
  template<int NR, int NC, typename CharT>
  void draw_texture(ScreenHandler<NR, NC, CharT>& sh,
                    int r, int c,
                    const Texture& texture,
                    const RC& tex_offset = { 0, 0 })
  {
    //draw_box_textured(sh,
    //                  r - 1, c - 1, texture.size.r + 2, texture.size.c + 2,
    //                  t8x::SolarDirection::Zenith,
    //                  texture);
    for (int r_idx = 0; r_idx < texture.size.r; ++r_idx)
    {
      for (int c_idx = 0; c_idx < texture.size.c; ++c_idx)
      {
        auto t = texture(r_idx + tex_offset.r, c_idx + tex_offset.c);
        sh.write_buffer(t.glyph, r_idx + r, c_idx + c, t.get_style());
      }
    }
  }
  
  // #NOTE: "Unwalled box".
  template<int NR, int NC, typename CharT>
  void draw_texture(ScreenHandler<NR, NC, CharT>& sh,
                    const RC& pos,
                    const Texture& texture,
                    const RC& tex_offset = { 0, 0 })
  {
    draw_texture(sh, pos.r, pos.c, texture, tex_offset);
  }
  
  // E.g.
  // r = 5, c = 6, len_r = 9, len_c = 7,
  // fill_texture.size.r = 9, fill_texture.size.c = 7,
  // shadow_texture.size.r = 9, shadow_texture.size.c = 6.
  template<int NR, int NC, typename CharT>
  void draw_box_texture_materials(ScreenHandler<NR, NC, CharT>& sh,
                                  int r, int c, int len_r, int len_c,
                                  const Texture& texture = {})
  {
    for (int i = r + 1; i < r + len_r - 1; ++i)
    {
      auto r0 = i - r;
      for (int j = 1; j < len_c - 1; ++j)
      {
        auto textel = texture(r0 - 1, j - 1);
        sh.write_buffer(textel.raw_mat_to_str(), i, j + c, textel.get_style());
      }
    }
  }
  
  template<int NR, int NC, typename CharT>
  void draw_box_texture_materials(ScreenHandler<NR, NC, CharT>& sh,
                                  const Rectangle& bb,
                                  const Texture& texture = {})
  {
    draw_box_texture_materials(sh,
                               bb.r, bb.c, bb.r_len, bb.c_len,
                               texture);
  }
  
  std::vector<RC> filled_circle_positions(const RC& center, float radius, float px_aspect)
  {
    std::vector<RC> positions;
    int r_offs = math::roundI(radius/px_aspect);
    int c_offs = math::roundI(radius);
    auto radius_sq = math::sq(radius);
    for (int r = -r_offs; r <= r_offs; ++r)
    {
      for (int c = -c_offs; c <= c_offs; ++c)
      {
        auto dist_sq = math::length_squared(r*px_aspect, static_cast<float>(c));
        if (dist_sq <= radius_sq)
          positions.emplace_back(center.r + r, center.c + c);
      }
    }
    return positions;
  }
  
  std::vector<RC> filled_arc_positions(const RC& center, float radius, float angle_rad,
                                       float dir_r, float dir_c, float px_aspect)
  {
    auto f_normalize_angle = [](float& ang)
    {
      while (ang < 0.f)
        ang += math::c_2pi;
      while (ang >= math::c_2pi)
        ang -= math::c_2pi;
    };
    
    std::vector<RC> positions;
    int r_offs = math::roundI(radius/px_aspect);
    int c_offs = math::roundI(radius);
    auto radius_sq = math::sq(radius);
    // Rotating dir vector CW and CCW using a rotation matrix.
    auto a = angle_rad*0.5f;
    auto Clo = std::cos(-a);
    auto Slo = std::sin(-a);
    auto Chi = std::cos(+a);
    auto Shi = std::sin(+a);
    math::normalize(dir_r, dir_c);
    float dir_lo_r = (dir_r*Clo - dir_c*Slo);
    float dir_lo_c = dir_r*Slo + dir_c*Clo;
    float dir_hi_r = (dir_r*Chi - dir_c*Shi);
    float dir_hi_c = dir_r*Shi + dir_c*Chi;
    
    auto lo_angle_rad = std::atan2(-dir_lo_r, dir_lo_c);
    auto hi_angle_rad = std::atan2(-dir_hi_r, dir_hi_c);
    f_normalize_angle(lo_angle_rad);
    f_normalize_angle(hi_angle_rad);
    if (lo_angle_rad > hi_angle_rad)
      hi_angle_rad += math::c_2pi;
    
    for (int r = -r_offs; r <= r_offs; ++r)
    {
      for (int c = -c_offs; c <= c_offs; ++c)
      {
        auto dist_sq = math::length_squared(r*px_aspect, static_cast<float>(c));
        if (dist_sq <= radius_sq)
        {
          auto curr_dir_r = static_cast<float>(r);
          auto curr_dir_c = static_cast<float>(c);
          math::normalize(curr_dir_r, curr_dir_c);
          auto curr_angle_rad = std::atan2(-curr_dir_r, curr_dir_c);
          f_normalize_angle(curr_angle_rad);
          // Make sure curr_angle_rad at least have a chance of being in the lo/hi angle range.
          if (curr_angle_rad < lo_angle_rad)
            curr_angle_rad += math::c_2pi;
            
          if (math::in_range(curr_angle_rad, lo_angle_rad, hi_angle_rad, Range::Closed))
            positions.emplace_back(center.r + r, center.c + c);
        }
      }
    }
    return positions;
  }
  
}
