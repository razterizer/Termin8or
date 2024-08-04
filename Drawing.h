#pragma once
#include <Core/Math.h>
#include <Core/bool_vector.h>
#include "Styles.h"
#include "Texture.h"

// Bresenham Algorithm.
namespace bresenham
{
  template<int NR, int NC>
  void plot_line_low(SpriteHandler<NR, NC>& sh, float x0, float y0, float x1, float y1,
                     const std::string& str, Color fg_color, Color bg_color)
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
                      const std::string& str, Color fg_color, Color bg_color)
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
                 const std::string& str, Color fg_color, Color bg_color)
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

// ////////////////////////////////
 
namespace drawing
{

  enum class OutlineType { Line, Masonry, Masonry2, Masonry3, Masonry4, Temple, Hash, NUM_ITEMS };
  enum class SolarDirection
  {
    Nadir, Zenith,
    N, NE, E, SE, S, SW, W, NW,
    N_Low, NE_Low, E_Low, SE_Low, S_Low, SW_Low, W_Low, NW_Low,
    NUM_ITEMS
  };
  
  template<int NR, int NC>
  void draw_box_outline(SpriteHandler<NR, NC>& sh,
                        int r, int c, int len_r, int len_c,
                        OutlineType outline_type,
                        const styles::Style& outline_style = { Color::Default, Color::Transparent2 },
                        const bool_vector& light_field = {})
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
    
    auto f_has_light = [&](int r0, int c0)
    {
      if (light_field.empty())
        return false;
      return static_cast<bool>(light_field[r0 * (len_c + 1) + c0]);
    };
    
    auto f_shade_style = [&](const styles::Style& style, int r0, int c0)
    {
      return shade_style(style, f_has_light(r0, c0) ?
          color::ShadeType::Bright : color::ShadeType::Unchanged, true);
    };
    
    // Outline
    int num_horiz = len_c + 1;
    int num_horiz_inset = num_horiz - 2;
    auto str_horiz_n = outline_nw + str::rep_char(outline_n, num_horiz_inset) + (len_c >= 1 ? outline_ne : "");
    auto str_horiz_s = outline_sw + str::rep_char(outline_s, num_horiz_inset) + (len_c >= 1 ? outline_se : "");

    for (int j = 0; j <= len_c; ++j)
    {
      sh.write_buffer(std::string(1, str_horiz_n[j]), r, j + c, f_shade_style(outline_style, 0, j));
      sh.write_buffer(std::string(1, str_horiz_s[j]), r + len_r, j + c, f_shade_style(outline_style, len_r, j));
    }
    for (int i = r + 1; i <= r + len_r - 1; ++i)
    {
      auto r0 = i - r;
      sh.write_buffer(outline_w, i, c, f_shade_style(outline_style, r0, 0));
      sh.write_buffer(outline_e, i, c + len_c, f_shade_style(outline_style, r0, len_c));
    }
  }
  
  template<int NR, int NC>
  void draw_box_outline(SpriteHandler<NR, NC>& sh,
                        const ttl::Rectangle& bb,
                        OutlineType outline_type,
                        const styles::Style& outline_style = { Color::Default, Color::Transparent2 },
                        const bool_vector& light_field = {})
  {
    draw_box_outline(sh, bb.r, bb.c, bb.r_len, bb.c_len, outline_type, outline_style, light_field);
  }
  
  // len_r = 0, len_c = 0 yields a 1x1 rectangle.
  template<int NR, int NC>
  void draw_box(SpriteHandler<NR, NC>& sh,
                int r, int c, int len_r, int len_c,
                const styles::Style& fill_style = { Color::Default, Color::Transparent2 },
                char fill_char = ' ',
                SolarDirection shadow_type = SolarDirection::Zenith,
                const styles::Style& shadow_style = { Color::Default, Color::Transparent2 },
                char shadow_char = ' ',
                const bool_vector& light_field = {})
  {
  
    auto f_has_light = [&](int r0, int c0)
    {
      if (light_field.empty())
        return false;
      return static_cast<bool>(light_field[r0 * (len_c + 1) + c0]);
    };
    
    auto f_shade_style = [&](const styles::Style& style, int r0, int c0)
    {
      return shade_style(style, f_has_light(r0, c0) ?
          color::ShadeType::Bright : color::ShadeType::Unchanged, true);
    };
    
    // Filling
    auto str_fill = std::string(1, fill_char); //str::rep_char(fill_char, num_horiz_inset);
    auto str_shadow_ns = std::string(1, shadow_char);//str::rep_char(shadow_char, num_horiz_inset);
    auto str_shadow_ew = std::string(1, shadow_char);
    
    if (len_r >= 2)
    {
      for (int i = 1; i <= len_c - 1; ++i)
      {
        if (shadow_type == SolarDirection::NW || shadow_type == SolarDirection::N || shadow_type == SolarDirection::NE)
          sh.write_buffer(str_shadow_ns, r + 1, i + c, f_shade_style(shadow_style, 1, i));
        else if (shadow_type == SolarDirection::SW || shadow_type == SolarDirection::S || shadow_type == SolarDirection::SE)
          sh.write_buffer(str_shadow_ns, r + len_r - 1, i + c, f_shade_style(shadow_style, len_r - 1, i));
        else if (shadow_type == SolarDirection::SW_Low || shadow_type == SolarDirection::S_Low || shadow_type == SolarDirection::SE_Low)
          sh.write_buffer(str_shadow_ns, r + 1, i + c, f_shade_style(fill_style, 1, i));
        else if (shadow_type == SolarDirection::NW_Low || shadow_type == SolarDirection::N_Low || shadow_type == SolarDirection::NE_Low)
          sh.write_buffer(str_shadow_ns, r + len_r - 1, i + c, f_shade_style(fill_style, len_r - 1, i));
      }
    }
    
    bool has_west_shadow = len_c >= 2 && (shadow_type == SolarDirection::SW || shadow_type == SolarDirection::W || shadow_type == SolarDirection::NW);
    bool has_east_shadow = len_c >= 2 && (shadow_type == SolarDirection::SE || shadow_type == SolarDirection::E || shadow_type == SolarDirection::NE);
    bool has_west_twilight = len_c >= 2 && (shadow_type == SolarDirection::SE_Low || shadow_type == SolarDirection::E_Low || shadow_type == SolarDirection::NE_Low);
    bool has_east_twilight = len_c >= 2 && (shadow_type == SolarDirection::SW_Low || shadow_type == SolarDirection::W_Low || shadow_type == SolarDirection::NW_Low);
    
    for (int i = r + 1; i <= r + len_r - 1; ++i)
    {
      auto r0 = i - r;
      if (has_west_shadow)
        sh.write_buffer(str_shadow_ew, i, c + 1, f_shade_style(shadow_style, r0, 1));
      else if (has_east_shadow)
        sh.write_buffer(str_shadow_ew, i, c + len_c - 1, f_shade_style(shadow_style, r0, len_c - 1));
      else if (has_west_twilight)
        sh.write_buffer(str_shadow_ew, i, c + 1, f_shade_style(fill_style, r0, 1));
      else if (has_east_twilight)
        sh.write_buffer(str_shadow_ew, i, c + len_c - 1, f_shade_style(fill_style, r0, len_c - 1));
      
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
      for (int j = 1; j <= len_c - 1; ++j)
        sh.write_buffer(str_fill, i, j + c, f_shade_style(style, r0, j));
    }
  }
  
  template<int NR, int NC>
  void draw_box(SpriteHandler<NR, NC>& sh,
                const ttl::Rectangle& bb,
                const styles::Style& fill_style = { Color::Default, Color::Transparent2 },
                char fill_char = ' ',
                SolarDirection shadow_type = SolarDirection::Zenith,
                const styles::Style& shadow_style = { Color::Default, Color::Transparent2 },
                char shadow_char = ' ',
                const bool_vector& light_field = {})
  {
    draw_box(sh,
             bb.r, bb.c, bb.r_len, bb.c_len,
             fill_style,
             fill_char,
             shadow_type,
             shadow_style,
             shadow_char,
             light_field);
  }
  
  // E.g.
  // r = 5, c = 6, len_r = 10, len_c = 8,
  // fill_texture.size.r = 9, fill_texture.size.c = 7,
  // shadow_texture.size.r = 9, shadow_texture.size.c = 6.
  template<int NR, int NC>
  void draw_box_textured(SpriteHandler<NR, NC>& sh,
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
      return static_cast<bool>(light_field[r0 * (len_c + 1) + c0]);
    };
    
    // Filling
    if (len_r >= 2)
    {
      for (int i = 1; i <= len_c - 1; ++i)
      {
        if (shadow_type == SolarDirection::NW || shadow_type == SolarDirection::N || shadow_type == SolarDirection::NE)
        {
          const auto tex_pos = tex_offset + RC { 0, i - 1 };
          const auto nt = fill_texture(tex_pos);
          const auto st = shadow_texture(tex_pos);
          const auto& t = f_has_light(1, i) ? nt : st;
          sh.write_buffer(t.str(), r + 1, i + c, t.get_style());
        }
        else if (shadow_type == SolarDirection::SW || shadow_type == SolarDirection::S || shadow_type == SolarDirection::SE)
        {
          const auto tex_pos = tex_offset + RC { len_r - 2, i - 1 };
          const auto nt = fill_texture(tex_pos);
          const auto st = shadow_texture(tex_pos);
          const auto& t = f_has_light(len_r - 1, i) ? nt : st;
          sh.write_buffer(t.str(), r + len_r - 1, i + c, t.get_style());
        }
        else if (shadow_type == SolarDirection::SW_Low || shadow_type == SolarDirection::S_Low || shadow_type == SolarDirection::SE_Low)
        {
          const auto tex_pos = tex_offset + RC { 0, i - 1 };
          const auto nt = fill_texture(tex_pos);
          sh.write_buffer(nt.str(), r + 1, i + c, nt.get_style());
        }
        else if (shadow_type == SolarDirection::NW_Low || shadow_type == SolarDirection::N_Low || shadow_type == SolarDirection::NE_Low)
        {
          const auto tex_pos = tex_offset + RC { len_r - 2, i - 1 };
          const auto nt = fill_texture(tex_pos);
          sh.write_buffer(nt.str(), r + len_r - 1, i + c, nt.get_style());
        }
      }
    }

    bool has_west_shadow = len_c >= 2 && (shadow_type == SolarDirection::SW || shadow_type == SolarDirection::W || shadow_type == SolarDirection::NW);
    bool has_east_shadow = len_c >= 2 && (shadow_type == SolarDirection::SE || shadow_type == SolarDirection::E || shadow_type == SolarDirection::NE);
    bool has_west_twilight = len_c >= 2 && (shadow_type == SolarDirection::SE_Low || shadow_type == SolarDirection::E_Low || shadow_type == SolarDirection::NE_Low);
    bool has_east_twilight = len_c >= 2 && (shadow_type == SolarDirection::SW_Low || shadow_type == SolarDirection::W_Low || shadow_type == SolarDirection::NW_Low);
    
    for (int i = r + 1; i <= r + len_r - 1; ++i)
    {
      auto r0 = i - r;
      if (has_west_shadow)
      {
        const auto tex_pos = tex_offset + RC { r0 - 1, 0 };
        const auto nt = fill_texture(tex_pos);
        const auto st = shadow_texture(tex_pos);
        const auto& t = f_has_light(r0, 1) ? nt : st;
        sh.write_buffer(t.str(), i, c + 1, t.get_style());
      }
      else if (has_east_shadow)
      {
        const auto tex_pos = tex_offset + RC { r0 - 1, len_c - 2 };
        const auto nt = fill_texture(tex_pos);
        const auto st = shadow_texture(tex_pos);
        const auto& t = f_has_light(r0, len_c - 1) ? nt : st;
        sh.write_buffer(t.str(), i, c + len_c - 1, t.get_style());
      }
      else if (has_west_twilight)
      {
        const auto tex_pos = tex_offset + RC { r0 - 1, 0 };
        const auto nt = fill_texture(tex_pos);
        sh.write_buffer(nt.str(), i, c + 1, nt.get_style());
      }
      else if (has_east_twilight)
      {
        const auto tex_pos = tex_offset + RC { r0 - 1, len_c - 2 };
        const auto nt = fill_texture(tex_pos);
        sh.write_buffer(nt.str(), i, c + len_c - 1, nt.get_style());
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
      for (int j = 1; j <= len_c - 1; ++j)
      {
        const auto tex_pos = tex_offset + RC { r0 - 1, j - 1 };
        const auto nt = fill_texture(tex_pos);
        const auto st = shadow_texture(tex_pos);
        const auto& t = f_has_light(r0, j) ? nt : ((is_underground || night_or_twilight) ? st : nt);
        sh.write_buffer(t.str(), i, j + c, t.get_style());
      }
    }
  }
  
  template<int NR, int NC>
  void draw_box_textured(SpriteHandler<NR, NC>& sh,
                         const ttl::Rectangle& bb,
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
  
  // E.g.
  // r = 5, c = 6, len_r = 10, len_c = 8,
  // fill_texture.size.r = 9, fill_texture.size.c = 7,
  // shadow_texture.size.r = 9, shadow_texture.size.c = 6.
  template<int NR, int NC>
  void draw_box_texture_materials(SpriteHandler<NR, NC>& sh,
                                  int r, int c, int len_r, int len_c,
                                  const Texture& texture = {})
  {
    for (int i = r + 1; i <= r + len_r - 1; ++i)
    {
      auto r0 = i - r;
      for (int j = 1; j <= len_c - 1; ++j)
      {
        auto textel = texture(r0 - 1, j - 1);
        sh.write_buffer(textel.mat_to_char_str(), i, j + c, textel.get_style());
      }
    }
  }
  
  template<int NR, int NC>
  void draw_box_texture_materials(SpriteHandler<NR, NC>& sh,
                                  const ttl::Rectangle& bb,
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
        auto dist_sq = math::length_squared<float>(r/px_aspect, c);
        if (dist_sq <= radius_sq)
          positions.emplace_back(center.r + r, center.c + c);
      }
    }
    return positions;
  }
  
}
