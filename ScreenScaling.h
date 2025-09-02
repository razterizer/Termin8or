//
//  ScreenScaling.h
//  Termin8or
//
//  Created by Rasmus Anthin on 2025-09-01.
//

#pragma once
#include "Color.h"
#include <map>

namespace screen_scaling
{

  struct RGBA
  {
    RGBA(double v) : r(v), g(v), b(v), a(v) {}
    RGBA(double rv, double gv, double bv) : r(rv), g(gv), b(bv), a(1) {}
    RGBA(double rv, double gv, double bv, double av) : r(rv), g(gv), b(bv), a(av) {}
    double r = 0, g = 0, b = 0, a = 0;
    RGBA operator+(const RGBA& other) { return { r+other.r, g+other.g, b+other.b, a+other.a }; }
    RGBA operator*(const RGBA& other) { return { r*other.r, g*other.g, b*other.b, a*other.a }; }
    RGBA operator*(double v) { return { r*v, g*v, b*v, a*v }; };
  };
  inline RGBA operator*(double v, const RGBA& rgba) { return { v*rgba.r, v*rgba.g, v*rgba.b, v*rgba.a }; }
  
  double clamp(double t) { return math::clamp(t, 0., 1.); }
  RGBA clamp(const RGBA& t) { return RGBA { math::clamp(t.r, 0., 1.), math::clamp(t.g, 0., 1.), math::clamp(t.b, 0., 1.), math::clamp(t.a, 0., 1.) }; }
  
// ////////////////////////////////////////////////

  std::map<Color, RGBA> color2rgba
  {
    { Color::White, RGBA { 1, 1, 1 } },
    { Color::Cyan, RGBA { 0, 1, 1 } },
    { Color::Magenta, RGBA { 1, 0, 1 } },
    { Color::Blue, RGBA { 0, 0, 1 } },
    { Color::Yellow, RGBA { 1, 1, 0 } },
    { Color::Green, RGBA { 0, 1, 0 } },
    { Color::Red, RGBA { 1, 0, 0 } },
    { Color::LightGray, RGBA { 0.8, 0.8, 0.8 } },
    { Color::DarkGray, RGBA { 0.4, 0.4, 0.4 } },
    { Color::DarkCyan, RGBA { 0, 0.4, 0.4 } },
    { Color::DarkMagenta, RGBA { 0.4, 0, 0.4 } },
    { Color::DarkBlue, RGBA { 0, 0, 0.4 } },
    { Color::DarkYellow, RGBA { 0.4, 0.4, 0 } },
    { Color::DarkGreen, RGBA { 0, 0.4, 0 } },
    { Color::DarkRed, RGBA { 0.4, 0, 0 } },
    { Color::Black, RGBA { 0, 0, 0 } },
    { Color::Transparent2, RGBA { 0, 0, 0, 0 } },
  };

  char find_closest_val(double shading_value) const
  {
    // Character set representing different shades
    const std::string shading_charset = "$#@&%*+=)(\\/\":^_-,. ";
    double t = shading_value;
    auto index = static_cast<int>((1-t) * (shading_charset.size() - 1));
    return shading_charset[index];
  }

  Color find_closest_val(RGBA shading_value) const
  {
    // Character set representing different shades
    auto t = shading_value;
    auto distance_squared = [&](Color color, const RGBA& rgba1)
    {
      auto it = color2rgba.find(color);
      if (it != color2rgba.end())
      {
        auto rgba0 = it->second;
        return math::distance<double>(rgba0.r, rgba0.g, rgba0.b, rgba0.a,
                                      rgba1.r, rgba1.g, rgba1.b, rgba1.a);
      }
      return math::get_max<double>();
    };
    for (int color = static_cast<int>(Color::Black); color <= static_cast<int>(Color::White); ++color)
      if (distance_squared(static_cast<Color>(color), t) < 0.5)
        return static_cast<Color>(color);
    return Color::Transparent2;
  }
  
  double find_closest_shading_value(char c) const
  {
    switch (c)
    {
      case '$': return 1.0;
      case '#': return 0.95;
      case '@': return 0.9;
      case '&': return 0.85;
      case '%': return 0.8;
      case '*': return 0.75;
      case '+': return 0.7;
      case '=': return 0.6;
      case ')': return 0.5;
      case '(': return 0.48;
      case '\\': return 0.47;
      case '/': return 0.46;
      case '|': return 0.45;
      case '\"': return 0.44;
      case ':': return 0.3;
      case '^': return 0.2;
      case '_': return 0.18;
      case '-': return 0.16;
      case ',': return 0.12;
      case '.': return 0.1;
      case ' ': return 0.0;
      default: return 0.5;
    }
  }

  RGBA find_closest_shading_value(Color color) const
  {
    auto col_idx = static_cast<int>(color);
    if (math::in_range<int>(col_idx, static_cast<int>(Color::Black), static_cast<int>(Color::White), Range::Closed))
    {
      auto it = color2rgba.find(color);
      if (it != color2rgba.end())
        return it->second;
    }
    return { 0, 0, 0, 0 }; // Transparent2
  }
  
  template<typename T, typename P>
  T bilinear_interpolate(std::array<std::array<T, NC>, NR>& buffer,
                         double row, double col) const
  {
    int top = static_cast<int>(row);
    int bottom = top + 1;
    int left = static_cast<int>(col);
    int right = left + 1;
    
    double top_weight = bottom - row; // "1"
    double bottom_weight = row - top; // "0"
    double left_weight = right - col; // "1"
    double right_weight = col - left; // "0"
    
    //double den = top_weight * left_weight + top_weight * right_weight + bottom_weight * left_weight + bottom_weight * right_weight;
    //double den = 1;
    
    P shading_value = static_cast<P>(0);
    const P zero_shading_value = shading_value;
    auto top_valid = 0 <= top && top < NR;
    auto btm_valid = 0 <= bottom && bottom < NR;
    auto lft_valid = 0 <= left && left < NC;
    auto rgt_valid = 0 <= right && right < NC;
    auto val_top_lft = top_valid && lft_valid ? buffer[top][left] : find_closest_val(zero_shading_value);
    auto val_top_rgt = top_valid && rgt_valid ? buffer[top][right] : find_closest_val(zero_shading_value);
    auto val_btm_lft = btm_valid && lft_valid ? buffer[bottom][left] : find_closest_val(zero_shading_value);
    auto val_btm_rgt = btm_valid && rgt_valid ? buffer[bottom][right] : find_closest_val(zero_shading_value);
    auto sh_top_lft = find_closest_shading_value(val_top_lft);
    auto sh_top_rgt = find_closest_shading_value(val_top_rgt);
    auto sh_btm_lft = find_closest_shading_value(val_btm_lft);
    auto sh_btm_rgt = find_closest_shading_value(val_btm_rgt);
    shading_value =
    (top_weight * left_weight * sh_top_lft +
     top_weight * right_weight * sh_top_rgt +
     bottom_weight * left_weight * sh_btm_lft +
     bottom_weight * right_weight * sh_btm_rgt); // / den;
    
    shading_value = clamp(shading_value);
    
    T value = find_closest_val(shading_value);
    
    return value;
  }
  
  // Function to downsample graphics using bilinear interpolation
  template<typename T, typename P, int NRo, int NCo>
  std::array<std::array<T, NCo>, NRo> resample_data(std::array<std::array<T, NC>, NR>& buffer, double offs) const
  {
    double row_ratio = static_cast<double>(NR) / NRo;
    double col_ratio = static_cast<double>(NC) / NCo;
    
    std::array<std::array<T, NCo>, NRo> buffer_new;
    
    for (int i = 0; i < NRo; ++i)
    {
      for (int j = 0; j < NCo; ++j)
      {
        double original_row = i * row_ratio;
        double original_col = j * col_ratio;
        
        T interpolated_val = bilinear_interpolate<T, P>(buffer,
                                                        original_row + offs,
                                                        original_col + offs);
        
        buffer_new[i][j] = interpolated_val;
      }
    }
    return buffer_new;
  }


// ////////////////////////////////////////////////

  template<int NRo, int NCo, int NRi, int NCi>
  friend void screen_scaling::resample(const ScreenHandler<NRi, NCi>& sh_src,
                                       ScreenHandler<NRo, NCo>& sh_dst)
  {
    auto new_screen_buffer = resample_data<char, double, NRo, NCo>(sh_src.screen_buffer, 0);
    auto new_fg_color_buffer = resample_data<Color, RGBA, NRo, NCo>(sh_src.fg_color_buffer, 0);
    auto new_bg_color_buffer = resample_data<Color, RGBA, NRo, NCo>(sh_src.bg_color_buffer, 0);
    sh_dst.overwrite_data(new_screen_buffer, new_fg_color_buffer, new_bg_color_buffer);
  }
  
}
