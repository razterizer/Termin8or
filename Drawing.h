#pragma once
#include <Core/Math.h>
#include <Core/bool_vector.h>
#include "Styles.h"
#include "Texture.h"

// Bresenham Algorithm.
namespace bresenham
{
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
  
  void scan_line(float r0, float c0, float r1, float c1, float r, float* c)
  {
    // r = k*c + m.
    auto k = (r1 - r0)/(c1 - c0);
    auto m = r0 - k*c0;
    if (c != nullptr)
      *c = (r - m)/k;
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

  template<int NR, int NC>
  void plot_line(ScreenHandler<NR, NC>& sh, float r0, float c0, float r1, float c1,
                 const std::string& str, Color fg_color, Color bg_color)
  {
    std::vector<RC> points;
    plot_line(r0, c0, r1, c1, points);
    for (const auto& pt : points)
      sh.write_buffer(str, pt.r, pt.c, fg_color, bg_color);
  }
  
  template<int NR, int NC>
  void plot_line(ScreenHandler<NR, NC>& sh, const RC& p0, const RC& p1,
                 const std::string& str, Color fg_color, Color bg_color)
  {
    std::vector<RC> points;
    plot_line(p0, p1, points);
    for (const auto& pt : points)
      sh.write_buffer(str, pt.r, pt.c, fg_color, bg_color);
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
  void draw_box_outline(ScreenHandler<NR, NC>& sh,
                        int r, int c, int len_r, int len_c,
                        OutlineType outline_type,
                        const styles::Style& outline_style = { Color::Default, Color::Transparent2 },
                        const bool_vector& light_field = {})
  {
    // len_r = 4, len_c = 3
    // ###
    // # #
    // # #
    // ###
    
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
      return static_cast<bool>(light_field[r0 * len_c + c0]);
    };
    
    auto f_shade_style = [&](const styles::Style& style, int r0, int c0)
    {
      return shade_style(style, f_has_light(r0, c0) ?
          color::ShadeType::Bright : color::ShadeType::Unchanged, true);
    };
    
    // Outline
    int num_horiz = len_c;
    int num_horiz_inset = num_horiz - 2;
    auto str_horiz_n = outline_nw + str::rep_char(outline_n, num_horiz_inset) + (len_c >= 2 ? outline_ne : "");
    auto str_horiz_s = outline_sw + str::rep_char(outline_s, num_horiz_inset) + (len_c >= 2 ? outline_se : "");

    for (int j = 0; j < len_c; ++j)
    {
      sh.write_buffer(std::string(1, str_horiz_n[j]), r, j + c, f_shade_style(outline_style, 0, j));
      sh.write_buffer(std::string(1, str_horiz_s[j]), r + len_r - 1, j + c, f_shade_style(outline_style, len_r - 1, j));
    }
    for (int i = r + 1; i < r + len_r - 1; ++i)
    {
      auto r0 = i - r;
      sh.write_buffer(outline_w, i, c, f_shade_style(outline_style, r0, 0));
      sh.write_buffer(outline_e, i, c + len_c - 1, f_shade_style(outline_style, r0, len_c - 1));
    }
  }
  
  template<int NR, int NC>
  void draw_box_outline(ScreenHandler<NR, NC>& sh,
                        const ttl::Rectangle& bb,
                        OutlineType outline_type,
                        const styles::Style& outline_style = { Color::Default, Color::Transparent2 },
                        const bool_vector& light_field = {})
  {
    draw_box_outline(sh, bb.r, bb.c, bb.r_len, bb.c_len, outline_type, outline_style, light_field);
  }
  
  template<int NR, int NC>
  void draw_box(ScreenHandler<NR, NC>& sh,
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
      return static_cast<bool>(light_field[r0 * len_c + c0]);
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
    
    if (len_r >= 3)
    {
      for (int i = 1; i < len_c - 1; ++i)
      {
        if (shadow_type == SolarDirection::NW || shadow_type == SolarDirection::N || shadow_type == SolarDirection::NE)
          sh.write_buffer(str_shadow_ns, r + 1, i + c, f_shade_style(shadow_style, 1, i));
        else if (shadow_type == SolarDirection::SW || shadow_type == SolarDirection::S || shadow_type == SolarDirection::SE)
          sh.write_buffer(str_shadow_ns, r + len_r - 2, i + c, f_shade_style(shadow_style, len_r - 2, i));
        else if (shadow_type == SolarDirection::SW_Low || shadow_type == SolarDirection::S_Low || shadow_type == SolarDirection::SE_Low)
          sh.write_buffer(str_shadow_ns, r + 1, i + c, f_shade_style(fill_style, 1, i));
        else if (shadow_type == SolarDirection::NW_Low || shadow_type == SolarDirection::N_Low || shadow_type == SolarDirection::NE_Low)
          sh.write_buffer(str_shadow_ns, r + len_r - 2, i + c, f_shade_style(fill_style, len_r - 2, i));
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
        sh.write_buffer(str_shadow_ew, i, c + 1, f_shade_style(shadow_style, r0, 1));
      else if (has_east_shadow)
        sh.write_buffer(str_shadow_ew, i, c + len_c - 2, f_shade_style(shadow_style, r0, len_c - 2));
      else if (has_west_twilight)
        sh.write_buffer(str_shadow_ew, i, c + 1, f_shade_style(fill_style, r0, 1));
      else if (has_east_twilight)
        sh.write_buffer(str_shadow_ew, i, c + len_c - 2, f_shade_style(fill_style, r0, len_c - 2));
      
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
        sh.write_buffer(str_fill, i, j + c, f_shade_style(style, r0, j));
    }
  }
  
  template<int NR, int NC>
  void draw_box(ScreenHandler<NR, NC>& sh,
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
  // r = 5, c = 6, len_r = 9, len_c = 7,
  // fill_texture.size.r = 9, fill_texture.size.c = 7,
  // shadow_texture.size.r = 9, shadow_texture.size.c = 6.
  template<int NR, int NC>
  void draw_box_textured(ScreenHandler<NR, NC>& sh,
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
          sh.write_buffer(t.str(), r + 1, i + c, t.get_style());
        }
        else if (shadow_type == SolarDirection::SW || shadow_type == SolarDirection::S || shadow_type == SolarDirection::SE)
        {
          const auto tex_pos = tex_offset + RC { len_r - 3, i - 1 };
          const auto nt = fill_texture(tex_pos);
          const auto st = shadow_texture(tex_pos);
          const auto& t = f_has_light(len_r - 2, i) ? nt : st;
          sh.write_buffer(t.str(), r + len_r - 2, i + c, t.get_style());
        }
        else if (shadow_type == SolarDirection::SW_Low || shadow_type == SolarDirection::S_Low || shadow_type == SolarDirection::SE_Low)
        {
          const auto tex_pos = tex_offset + RC { 0, i - 1 };
          const auto nt = fill_texture(tex_pos);
          sh.write_buffer(nt.str(), r + 1, i + c, nt.get_style());
        }
        else if (shadow_type == SolarDirection::NW_Low || shadow_type == SolarDirection::N_Low || shadow_type == SolarDirection::NE_Low)
        {
          const auto tex_pos = tex_offset + RC { len_r - 3, i - 1 };
          const auto nt = fill_texture(tex_pos);
          sh.write_buffer(nt.str(), r + len_r - 2, i + c, nt.get_style());
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
        sh.write_buffer(t.str(), i, c + 1, t.get_style());
      }
      else if (has_east_shadow)
      {
        const auto tex_pos = tex_offset + RC { r0 - 1, len_c - 3 };
        const auto nt = fill_texture(tex_pos);
        const auto st = shadow_texture(tex_pos);
        const auto& t = f_has_light(r0, len_c - 2) ? nt : st;
        sh.write_buffer(t.str(), i, c + len_c - 2, t.get_style());
      }
      else if (has_west_twilight)
      {
        const auto tex_pos = tex_offset + RC { r0 - 1, 0 };
        const auto nt = fill_texture(tex_pos);
        sh.write_buffer(nt.str(), i, c + 1, nt.get_style());
      }
      else if (has_east_twilight)
      {
        const auto tex_pos = tex_offset + RC { r0 - 1, len_c - 3 };
        const auto nt = fill_texture(tex_pos);
        sh.write_buffer(nt.str(), i, c + len_c - 2, nt.get_style());
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
        sh.write_buffer(t.str(), i, j + c, t.get_style());
      }
    }
  }
  
  template<int NR, int NC>
  void draw_box_textured(ScreenHandler<NR, NC>& sh,
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
  // r = 5, c = 6, len_r = 9, len_c = 7,
  // fill_texture.size.r = 9, fill_texture.size.c = 7,
  // shadow_texture.size.r = 9, shadow_texture.size.c = 6.
  template<int NR, int NC>
  void draw_box_texture_materials(ScreenHandler<NR, NC>& sh,
                                  int r, int c, int len_r, int len_c,
                                  const Texture& texture = {})
  {
    for (int i = r + 1; i < r + len_r - 1; ++i)
    {
      auto r0 = i - r;
      for (int j = 1; j < len_c - 1; ++j)
      {
        auto textel = texture(r0 - 1, j - 1);
        sh.write_buffer(textel.mat_to_char_str(), i, j + c, textel.get_style());
      }
    }
  }
  
  template<int NR, int NC>
  void draw_box_texture_materials(ScreenHandler<NR, NC>& sh,
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
            
          if (math::in_range<float>(curr_angle_rad, lo_angle_rad, hi_angle_rad, Range::Closed))
            positions.emplace_back(center.r + r, center.c + c);
        }
      }
    }
    return positions;
  }
  
}
