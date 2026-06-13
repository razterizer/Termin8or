//
//  ScreenScaling.h
//  Termin8or
//
//  Created by Rasmus Anthin on 2025-09-01.
//

#pragma once
#include "Color.h"
#include <map>

namespace t8x
{
  using Color = t8::Color;
  using Color16 = t8::Color16;

  double clamp(double t) { return math::clamp(t, 0., 1.); }
  t8::RGBA clamp(const t8::RGBA& t)
  {
    return t8::RGBA
    {
      math::clamp(t.r, 0., 1.),
      math::clamp(t.g, 0., 1.),
      math::clamp(t.b, 0., 1.),
      math::clamp(t.a, 0., 1.)
    };
  }

  template<typename FromT, typename ToT>
  ToT find_closest_val(FromT shading_value);
  
  template<>
  char find_closest_val(double shading_value)
  {
    // Character set representing different shades
    const std::string shading_charset = "$#@&%*+=)(\\/\":^_-,. ";
    double t = shading_value;
    auto index = static_cast<int>((1-t) * (shading_charset.size() - 1));
    return shading_charset[index];
  }
  
  template<>
  char32_t find_closest_val(double shading_value)
  {
    // Character set representing different shades
    const std::string shading_charset = "$#@&%*+=)(\\/\":^_-,. ";
    double t = shading_value;
    auto index = static_cast<int>((1-t) * (shading_charset.size() - 1));
    return shading_charset[index];
  }

  template<>
  Color find_closest_val(t8::RGBA shading_value)
  {
    // Character set representing different shades
    auto t = shading_value;
    auto distance_squared = [&](Color color, const t8::RGBA& rgba1)
    {
      auto it = t8::color2rgba.find(color);
      if (it != t8::color2rgba.end())
      {
        auto rgba0 = it->second;
        return math::distance_squared<double>(rgba0.r, rgba0.g, rgba0.b, rgba0.a,
                                              rgba1.r, rgba1.g, rgba1.b, rgba1.a);
      }
      return math::get_max<double>();
    };
    Color best_color = Color16::Default;
    auto min_dist_sq = math::get_max<double>();
    for (int col_idx = t8::c_min_color_idx; col_idx <= t8::c_max_color_idx; ++col_idx)
      if (math::minimize(min_dist_sq, distance_squared(Color(col_idx), t)))
        best_color = Color(col_idx);
    return best_color;
  }
  
  template<typename CharT>
  double find_closest_shading_value(CharT c)
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

  t8::RGBA find_closest_shading_value(Color color)
  {
    auto col_idx = color.get_index();
    if (math::in_range(col_idx, t8::c_min_color_idx, t8::c_max_color_idx, Range::Closed))
    {
      auto it = t8::color2rgba.find(color);
      if (it != t8::color2rgba.end())
        return it->second;
    }
    return { 0, 0, 0, 0 }; // Transparent2
  }
  
  template<typename T, typename P, int NR, int NC>
  T bilinear_interpolate(const T tl, const T tr, const T bl, const T br,
                         double row, double col)
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
    auto val_top_lft = top_valid && lft_valid ? tl : find_closest_val<P, T>(zero_shading_value);
    auto val_top_rgt = top_valid && rgt_valid ? tr : find_closest_val<P, T>(zero_shading_value);
    auto val_btm_lft = btm_valid && lft_valid ? bl : find_closest_val<P, T>(zero_shading_value);
    auto val_btm_rgt = btm_valid && rgt_valid ? br : find_closest_val<P, T>(zero_shading_value);
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
    
    T value = find_closest_val<P, T>(shading_value);
    
    return value;
  }
  
  // Function to downsample graphics using bilinear interpolation
  template<typename CharT, int NRi, int NCi, int NRo, int NCo>
  std::array<t8::BufferCell<CharT>, NRo*NCo> resample_data(const std::array<t8::BufferCell<CharT>, NRi*NCi>& buffer, double offs)
  {
    double row_ratio = (NRo > 1) ? static_cast<double>(NRi - 1) / (NRo - 1) : 0.0;
    double col_ratio = (NCo > 1) ? static_cast<double>(NCi - 1) / (NCo - 1) : 0.0;
    
    std::array<t8::BufferCell<CharT>, NCo*NRo> buffer_new;
    
    t8::BufferCell<CharT> empty_cell { ' ', Color16::Default, Color16::Transparent };
    
    for (int i = 0; i < NRo; ++i)
    {
      for (int j = 0; j < NCo; ++j)
      {
        double original_row = i * row_ratio;
        double original_col = j * col_ratio;
        
        double row = original_row + offs;
        double col = original_col + offs;
        
        int top = static_cast<int>(row);
        int bottom = top + 1;
        int left = static_cast<int>(col);
        int right = left + 1;
        
        auto top_valid = 0 <= top && top < NRi;
        auto btm_valid = 0 <= bottom && bottom < NRi;
        auto lft_valid = 0 <= left && left < NCi;
        auto rgt_valid = 0 <= right && right < NCi;
        
        const auto& tl = top_valid && lft_valid ? buffer[top*NCi + left] : empty_cell;
        const auto& tr = top_valid && rgt_valid ? buffer[top*NCi + right] : empty_cell;
        const auto& bl = btm_valid && lft_valid ? buffer[bottom*NCi + left] : empty_cell;
        const auto& br = btm_valid && rgt_valid ? buffer[bottom*NCi + right] : empty_cell;
        
        auto ch_interp =
          bilinear_interpolate<CharT, double, NRi, NCi>(tl.ch, tr.ch, bl.ch, br.ch,
                                                    row, col);
        auto fg_interp =
          bilinear_interpolate<t8::Color, t8::RGBA, NRi, NCi>(tl.fg, tr.fg, bl.fg, br.fg,
                                                      row, col);
        auto bg_interp =
          bilinear_interpolate<t8::Color, t8::RGBA, NRi, NCi>(tl.bg, tr.bg, bl.bg, br.bg,
                                                      row, col);
        
        buffer_new[i*NCo + j] = { ch_interp, fg_interp, bg_interp };
      }
    }
    return buffer_new;
  }


// ////////////////////////////////////////////////

  template<int NRo, int NCo, int NRi, int NCi, typename CharT>
  void screen_scaling::resample(const t8::ScreenHandler<NRi, NCi, CharT>& sh_src,
                                       t8::ScreenHandler<NRo, NCo, CharT>& sh_dst)
  {
    auto new_screen_buffer = resample_data<CharT, NRi, NCi, NRo, NCo>(sh_src.screen_buffer, 0);
    sh_dst.overwrite_data(new_screen_buffer);
  }
  
}
