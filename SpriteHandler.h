#pragma once
#include "Text.h"
#include "Rectangle.h"
#include "Styles.h"
#include <Core/StringHelper.h>
#include <Core/TextBox.h>
#include <Core/StlUtils.h>
#include <Core/Math.h>
#include <array>
#include <map>

struct OrderedText
{
  std::string str;
  int r = -1;
  int c = -1;
  int priority = 0;
  styles::Style style;
};

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

// //////////////////////////////////////////////////

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
      write_buffer(text.str, text.r, text.c, text.style.fg_color, text.style.bg_color);
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
  
  void overwrite_data(const std::array<std::array<char, NC>, NR>& new_screen_buffer,
                      const std::array<std::array<Text::Color, NC>, NR>& new_fg_color_buffer,
                      const std::array<std::array<Text::Color, NC>, NR>& new_bg_color_buffer)
  {
    screen_buffer = new_screen_buffer;
    fg_color_buffer = new_fg_color_buffer;
    bg_color_buffer = new_bg_color_buffer;
  }
  
  template<int NRo, int NCo>
  inline void resample_to(SpriteHandler<NRo, NCo>& sh_scaled)
  {
    auto new_screen_buffer = resample_data<char, double, NRo, NCo>(screen_buffer, 0);
    auto new_fg_color_buffer = resample_data<Text::Color, RGBA, NRo, NCo>(fg_color_buffer, 0);
    auto new_bg_color_buffer = resample_data<Text::Color, RGBA, NRo, NCo>(bg_color_buffer, 0);
    sh_scaled.overwrite_data(new_screen_buffer, new_fg_color_buffer, new_bg_color_buffer);
  }
  
private:
  std::map<Text::Color, RGBA> color2rgba
  {
    { Text::Color::White, RGBA { 1, 1, 1 } },
    { Text::Color::Cyan, RGBA { 0, 1, 1 } },
    { Text::Color::Magenta, RGBA { 1, 0, 1 } },
    { Text::Color::Blue, RGBA { 0, 0, 1 } },
    { Text::Color::Yellow, RGBA { 1, 1, 0 } },
    { Text::Color::Green, RGBA { 0, 1, 0 } },
    { Text::Color::Red, RGBA { 1, 0, 0 } },
    { Text::Color::LightGray, RGBA { 0.8, 0.8, 0.8 } },
    { Text::Color::DarkGray, RGBA { 0.4, 0.4, 0.4 } },
    { Text::Color::DarkCyan, RGBA { 0, 0.4, 0.4 } },
    { Text::Color::DarkMagenta, RGBA { 0.4, 0, 0.4 } },
    { Text::Color::DarkBlue, RGBA { 0, 0, 0.4 } },
    { Text::Color::DarkYellow, RGBA { 0.4, 0.4, 0 } },
    { Text::Color::DarkGreen, RGBA { 0, 0.4, 0 } },
    { Text::Color::DarkRed, RGBA { 0.4, 0, 0 } },
    { Text::Color::Black, RGBA { 0, 0, 0 } },
    { Text::Color::Transparent2, RGBA { 0, 0, 0, 0 } },
  };

  template<typename T, typename P>
  T find_closest_val(P shading_value) const { return static_cast<T>(shading_value); }
  template<> char find_closest_val(double shading_value) const
  {
    // Character set representing different shades
    const std::string shading_charset = "$#@&%*+=)(\\/\":^_-,. ";
    double t = shading_value;
    auto index = static_cast<int>((1-t) * (shading_charset.size() - 1));
    return shading_charset[index];
  }
  template<> Text::Color find_closest_val(RGBA shading_value) const
  {
    // Character set representing different shades
    auto t = shading_value;
    auto distance_squared = [&](Text::Color color, const RGBA& rgba1)
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
    for (int color = static_cast<int>(Text::Color::Black); color <= static_cast<int>(Text::Color::White); ++color)
      if (distance_squared(static_cast<Text::Color>(color), t) < 0.5)
        return static_cast<Text::Color>(color);
    return Text::Color::Transparent2;
  }
  
  template<typename T, typename P>
  P find_closest_shading_value(T v) const { return static_cast<P>(v); }
  template<> double find_closest_shading_value(char c) const
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
  template<> RGBA find_closest_shading_value(Text::Color color) const
  {
    auto col_idx = static_cast<int>(color);
    if (math::in_range<int>(col_idx, static_cast<int>(Text::Color::Black), static_cast<int>(Text::Color::White), Range::Closed))
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
    auto top_valid = 0 <= top && top < NR;
    auto btm_valid = 0 <= bottom && bottom < NR;
    auto lft_valid = 0 <= left && left < NC;
    auto rgt_valid = 0 <= right && right < NC;
    auto val_top_lft = top_valid && lft_valid ? buffer[top][left] : find_closest_val<T, P>(0);
    auto val_top_rgt = top_valid && rgt_valid ? buffer[top][right] : find_closest_val<T, P>(0);
    auto val_btm_lft = btm_valid && lft_valid ? buffer[bottom][left] : find_closest_val<T, P>(0);
    auto val_btm_rgt = btm_valid && rgt_valid ? buffer[bottom][right] : find_closest_val<T, P>(0);
    auto sh_top_lft = find_closest_shading_value<T, P>(val_top_lft);
    auto sh_top_rgt = find_closest_shading_value<T, P>(val_top_rgt);
    auto sh_btm_lft = find_closest_shading_value<T, P>(val_btm_lft);
    auto sh_btm_rgt = find_closest_shading_value<T, P>(val_btm_rgt);
    shading_value =
    (top_weight * left_weight * sh_top_lft +
     top_weight * right_weight * sh_top_rgt +
     bottom_weight * left_weight * sh_btm_lft +
     bottom_weight * right_weight * sh_btm_rgt); // / den;
    
    shading_value = clamp(shading_value);
    
    T value = find_closest_val<T, P>(shading_value);
    
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

};

