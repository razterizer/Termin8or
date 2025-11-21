//
//  Color.h
//  Termin8or
//
//  Created by Rasmus Anthin on 2024-06-20.
//

#pragma once
#include <Core/Rand.h>
#include <string>

namespace t8
{

  enum class Color16
  {
    Transparent = -1,
    Transparent2 = -2,
    Default = 0,
    Black,         //  1
    DarkRed,       //  2
    DarkGreen,     //  3
    DarkYellow,    //  4
    DarkBlue,      //  5
    DarkMagenta,   //  6
    DarkCyan,      //  7
    LightGray,     //  8
    DarkGray,      //  9
    Red,           // 10
    Green,         // 11
    Yellow,        // 12
    Blue,          // 13
    Magenta,       // 14
    Cyan,          // 15
    White          // 16
  };

  Color16 string_to_color(const std::string& str)
  {
    if (str == "Transparent")
      return Color16::Transparent;
    if (str == "Transparent2")
      return Color16::Transparent2;
    if (str == "Default")
      return Color16::Default;
    if (str == "Black")
      return Color16::Black;
    if (str == "DarkRed")
      return Color16::DarkRed;
    if (str == "DarkGreen")
      return Color16::DarkGreen;
    if (str == "DarkYellow")
      return Color16::DarkYellow;
    if (str == "DarkBlue")
      return Color16::DarkBlue;
    if (str == "DarkMagenta")
      return Color16::DarkMagenta;
    if (str == "DarkCyan")
      return Color16::DarkCyan;
    if (str == "LightGray")
      return Color16::LightGray;
    if (str == "DarkGray")
      return Color16::DarkGray;
    if (str == "Red")
      return Color16::Red;
    if (str == "Green")
      return Color16::Green;
    if (str == "Yellow")
      return Color16::Yellow;
    if (str == "Blue")
      return Color16::Blue;
    if (str == "Magenta")
      return Color16::Magenta;
    if (str == "Cyan")
      return Color16::Cyan;
    if (str == "White")
      return Color16::White;
      
    return Color16::Default;
  }
  
  std::string color_to_string(Color16 color)
  {
    switch (color)
    {
      case Color16::Transparent:
        return "Transparent";
      case Color16::Transparent2:
        return "Transparent2";
      case Color16::Default:
        return "Default";
      case Color16::Black:
        return "Black";
      case Color16::DarkRed:
        return "DarkRed";
      case Color16::DarkGreen:
        return "DarkGreen";
      case Color16::DarkYellow:
        return "DarkYellow";
      case Color16::DarkBlue:
        return "DarkBlue";
      case Color16::DarkMagenta:
        return "DarkMagenta";
      case Color16::DarkCyan:
        return "DarkCyan";
      case Color16::LightGray:
        return "LightGray";
      case Color16::DarkGray:
        return "DarkGray";
      case Color16::Red:
        return "Red";
      case Color16::Green:
        return "Green";
      case Color16::Yellow:
        return "Yellow";
      case Color16::Blue:
        return "Blue";
      case Color16::Magenta:
        return "Magenta";
      case Color16::Cyan:
        return "Cyan";
      case Color16::White:
        return "White";
      default:
        return "";
    }
  }
  
  std::vector<Color16> colors_hue_light
  {
    Color16::Red,
    Color16::Yellow,
    Color16::Green,
    Color16::Cyan,
    Color16::Blue,
  };
  
  std::vector<Color16> colors_hue_light_dark
  {
    Color16::Red,
    Color16::Yellow,
    Color16::Green,
    Color16::Cyan,
    Color16::Blue,
    Color16::Magenta,
    Color16::DarkRed,
    Color16::DarkYellow,
    Color16::DarkGreen,
    Color16::DarkCyan,
    Color16::DarkBlue,
    Color16::DarkMagenta,
  };
  
  // #NOTE: these color vectors are to be compared with each other, line by line.
  // These are not absolute categories! E.g. In colors_dark there is LightGray,
  //   but it is only there because it is darker than the corresponding White color
  //   in the colors_bright vector.
  std::vector<Color16> colors_bright
  {
    Color16::White,
    Color16::LightGray,
    Color16::DarkGray,
    Color16::Cyan,
    Color16::Magenta,
    Color16::Yellow,
    Color16::Red,
    Color16::Green,
    Color16::Blue,
  };
  std::vector<Color16> colors_dark
  {
    Color16::LightGray,
    Color16::DarkGray,
    Color16::Black,
    Color16::DarkCyan,
    Color16::DarkMagenta,
    Color16::DarkYellow,
    Color16::DarkRed,
    Color16::DarkGreen,
    Color16::DarkBlue,
  };
  
  enum class ShadeType { Unchanged, Bright, Dark };
  // #NOTE: Returns Color16::Default if no matching color was found.
  //   If shade = ShadeType::Unchanged, then it will return the same color as the argument.
  // If inputting a bright color and ShadeType is Bright, then you will get the same color.
  // If inputting a dark color and ShadeType is Bright, then you will get the corresponding bright color.
  // If inputting a bright color and ShadeType is Dark, then you will get the corresponding dark color.
  // If inputting a dark color and ShadeType is Dark, then you will get the same color.
  Color16 shade_color(Color16 color, ShadeType shade)
  {
    int idx = -1;
    switch (shade)
    {
      case ShadeType::Unchanged:
        return color;
      case ShadeType::Bright:
        idx = stlutils::find_idx(colors_dark, color);
        if (0 <= idx)
          return colors_bright[idx];
        if (stlutils::contains(colors_bright, color))
          return color;
        break;
      case ShadeType::Dark:
        idx = stlutils::find_idx(colors_bright, color);
        if (0 <= idx)
          return colors_dark[idx];
        if (stlutils::contains(colors_dark, color))
          return color;
        break;
    }
    return Color16::Default;
  }
  
  std::optional<bool> is_bright(Color16 color, bool perceived = false)
  {
    if (perceived)
    {
#ifndef _WIN32
      switch (color)
      {
        case Color16::DarkGreen: return true;
        case Color16::DarkYellow: return true;
        case Color16::DarkCyan: return true;
        case Color16::Red: return false;
        case Color16::Blue: return false;
        default:
          break;
      }
#endif
    }
    switch (color)
    {
      case Color16::Transparent:
      case Color16::Transparent2:
      case Color16::Default:
        return std::nullopt;
      case Color16::Black:
      case Color16::DarkRed:
      case Color16::DarkGreen:
      case Color16::DarkYellow:
      case Color16::DarkBlue:
      case Color16::DarkMagenta:
      case Color16::DarkCyan:
        return false;
      case Color16::LightGray: return true;
      case Color16::DarkGray: return false;
      case Color16::Red:
      case Color16::Green:
      case Color16::Yellow:
      case Color16::Blue:
      case Color16::Magenta:
      case Color16::Cyan:
      case Color16::White:
        return true;
    }
    assert(false && "Unhandled Color16 in is_bright()");
    return std::nullopt;
  }
  
  std::optional<bool> is_dark(Color16 color, bool perceived = false)
  {
    if (perceived)
    {
#ifndef _WIN32
      switch (color)
      {
        case Color16::DarkGreen: return false;
        case Color16::DarkYellow: return false;
        case Color16::DarkCyan: return false;
        case Color16::Red: return true;
        case Color16::Blue: return true;
        default:
          break;
      }
#endif
    }
    switch (color)
    {
      case Color16::Transparent:
      case Color16::Transparent2:
      case Color16::Default:
        return std::nullopt;
      case Color16::Black:
      case Color16::DarkRed:
      case Color16::DarkGreen:
      case Color16::DarkYellow:
      case Color16::DarkBlue:
      case Color16::DarkMagenta:
      case Color16::DarkCyan:
        return true;
      case Color16::LightGray: return false;
      case Color16::DarkGray: return true;
      case Color16::Red:
      case Color16::Green:
      case Color16::Yellow:
      case Color16::Blue:
      case Color16::Magenta:
      case Color16::Cyan:
      case Color16::White:
        return false;
    }
    assert(false && "Unhandled Color16 in is_dark()");
    return std::nullopt;
  }
  
  Color16 get_contrast_color(Color16 color)
  {
    switch (color)
    {
      case Color16::Transparent: return Color16::Transparent;
      case Color16::Transparent2: return Color16::Transparent2;
      case Color16::Default: return Color16::Default;
      case Color16::Black: return Color16::White;
      case Color16::DarkRed: return Color16::Cyan;
      case Color16::DarkGreen: return Color16::Magenta;
      case Color16::DarkYellow: return Color16::Blue;
      case Color16::DarkBlue: return Color16::Yellow;
      case Color16::DarkMagenta: return Color16::Green;
      case Color16::DarkCyan: return Color16::Red;
      case Color16::LightGray: return Color16::Black;
      case Color16::DarkGray: return Color16::White;
      case Color16::Red: return Color16::DarkCyan;
      case Color16::Green: return Color16::DarkMagenta;
      case Color16::Yellow: return Color16::DarkBlue;
      case Color16::Blue: return Color16::DarkYellow;
      case Color16::Magenta: return Color16::DarkGreen;
      case Color16::Cyan: return Color16::DarkRed;
      case Color16::White: return Color16::Black;
    }
    assert(false && "Unhandled Color16 in get_contrast_color()");
    return Color16::Default;
  }
  
  int get_color_value_win(Color16 color)
  {
    switch (color)
    {
      case Color16::DarkRed:
        return 4;
      case Color16::DarkGreen:
        return 2;
      case Color16::DarkYellow:
        return 6;
      case Color16::DarkBlue:
        return 1;
      case Color16::DarkMagenta:
        return 5;
      case Color16::DarkCyan:
        return 3;
      case Color16::LightGray:
        return 7;
      case Color16::DarkGray:
        return 8;
      case Color16::Red:
        return 12;
      case Color16::Green:
        return 10;
      case Color16::Yellow:
        return 14;
      case Color16::Blue:
        return 9;
      case Color16::Magenta:
        return 13;
      case Color16::Cyan:
        return 11;
      case Color16::White:
        return 15;
      case Color16::Black:
      default:
        return 0;
    }
  }
  
  Color16 get_color_win(int color_val)
  {
    switch (color_val)
    {
      case 0:
        return Color16::Black;
      case 1:
        return Color16::DarkBlue;
      case 2:
        return Color16::DarkGreen;
      case 3:
        return Color16::DarkCyan;
      case 4:
        return Color16::DarkRed;
      case 5:
        return Color16::DarkMagenta;
      case 6:
        return Color16::DarkYellow;
      case 7:
        return Color16::LightGray;
      case 8:
        return Color16::DarkGray;
      case 9:
        return Color16::Blue;
      case 10:
        return Color16::Green;
      case 11:
        return Color16::Cyan;
      case 12:
        return Color16::Red;
      case 13:
        return Color16::Magenta;
      case 14:
        return Color16::Yellow;
      case 15:
        return Color16::White;
      default:
        return Color16::Default;
    }
  }
  
  // ////////////////////////////////////////////////////////
  
  struct RGB6
  {
    uint8_t r = 0;
    uint8_t g = 0;
    uint8_t b = 0;
    
    constexpr RGB6() = default;
    constexpr RGB6(uint8_t r6, uint8_t g6, uint8_t b6)
      : r(std::min(r6, static_cast<uint8_t>(5)))
      , g(std::min(g6, static_cast<uint8_t>(5)))
      , b(std::min(b6, static_cast<uint8_t>(5)))
    {}
    
    std::tuple<int, int, int> to_int() const
    {
      return
      {
        static_cast<int>(r),
        static_cast<int>(g),
        static_cast<int>(b)
      };
    }
    
    std::tuple<float, float, float> to_float() const
    {
      return
      {
        r / 5.f,
        g / 5.f,
        b / 5.f
      };
    }
    
    std::tuple<float, float, float> to_xterm_float() const
    {
      static constexpr float L[6] =
      {
        0.0f,
        95.0f / 255.0f,
        135.0f / 255.0f,
        175.0f / 255.0f,
        215.0f / 255.0f,
        1.0f
      };
      
      return
      {
        L[r],
        L[g],
        L[b]
      };
    }
    
    bool operator==(const RGB6& rgb6) const
    {
      return rgb6.r == r && rgb6.g == g && rgb6.b == b;
    }
  };
  
  // ////////////////////////////////////////////////////////
  
  struct Gray24
  {
    uint8_t gray = 0;
    
    Gray24(uint8_t g)
      : gray(std::min(g, static_cast<uint8_t>(23)))
    {}
    
    constexpr float to_float() const
    {
      return gray / 23.f;
    }
    
    bool operator==(Gray24 g) const
    {
      return g.gray == gray;
    }
  };
  
  // ////////////////////////////////////////////////////////
  
  struct Color
  {
    Color() = default;
    Color(const Color& col)
      : idx(col.idx)
    {}
    explicit Color(int index)
      : idx(index)
    {}
    Color(Color16 col16)
      : idx(static_cast<int>(col16) - 1)
    {}
    Color(const RGB6& rgb6)
    {
      set_rgb(static_cast<int>(rgb6.r), static_cast<int>(rgb6.g), static_cast<int>(rgb6.b));
    }
    Color(int r, int g, int b)
    {
      assert(0 <= r && r < 6);
      assert(0 <= g && g < 6);
      assert(0 <= b && b < 6);
      r = std::clamp(r, 0, 5);
      g = std::clamp(g, 0, 5);
      b = std::clamp(b, 0, 5);
      set_rgb(r, g, b);
    }
    Color(float r, float g, float b)
    {
      assert(0.f <= r && r <= 1.f);
      assert(0.f <= g && g <= 1.f);
      assert(0.f <= b && b <= 1.f);
      r = std::clamp(r, 0.f, 1.f);
      g = std::clamp(g, 0.f, 1.f);
      b = std::clamp(b, 0.f, 1.f);
      set_rgb(math::roundI(5.f*r), math::roundI(5.f*g), math::roundI(5.f*b));
    }
    Color(Gray24 gray24)
    {
      idx = 232 + gray24.gray;
    }
    
    int get_index() const { return idx; }
    
    std::optional<Color16> try_get_color16() const
    {
      if (-3 <= idx && idx <= 15)
        return static_cast<Color16>(idx + 1);
      return std::nullopt;
    }

    std::optional<RGB6> try_get_rgb6() const
    {
      // Do not confuse with Color16::White = 16!
      // For idx = 0 .. 15, this corresponds to Color16::Black to Color16::White.
      if (16 <= idx && idx <= 231)
      {
        int v = idx - 16;
        int r = v / 36;
        int g = (v % 36) / 6;
        int b = v % 6;
        return RGB6
        {
          static_cast<uint8_t>(r),
          static_cast<uint8_t>(g),
          static_cast<uint8_t>(b)
        };
      }
      return std::nullopt;
    }
    
    std::optional<Gray24> try_get_gray24() const
    {
      if (232 <= idx && idx <= 255)
      {
        int v = idx - 232;
        return static_cast<uint8_t>(v);
      }
      return std::nullopt;
    }
    
  bool operator==(Color other) const { return this->idx == other.idx + 1; }
  bool operator==(Color16 col16) const { return this->idx + 1 == static_cast<int>(col16); }
  bool operator==(const RGB6& rgb6) const
  {
    auto val = try_get_rgb6();
    if (val.has_value())
      return val.value() == rgb6;
    return false;
  }
  bool operator==(Gray24 g24) const
  {
    auto val = try_get_gray24();
    if (val.has_value())
      return val.value() == g24;
    return false;
  }
  
  private:
    int idx = 0;
    
    void set_rgb(int r, int g, int b)
    {
      idx = 16 + 36*r + 6*g + b;
    }
  };
  
  inline bool operator==(Color16 col16, Color col) { return col == col16; }
  inline bool operator==(const RGB6& rgb6, Color col) { return col == rgb6; }
  inline bool operator==(Gray24 g24, Color col) { return col == g24; }
  
  // ////////////////////////////////////////////////////////
  
  // Returns Color16::Default if palette is empty. Otherwise identical to rnd::rand_select().
  Color get_random_color(const std::vector<Color>& palette)
  {
    auto num = static_cast<int>(palette.size());
    if (num == 0)
      return Color16::Default;
    auto idx = rnd::rand_int(0, num - 1);
    return palette[idx];
  }
  
  Color16 to_nearest_color16(Color col)
  {
    if (auto col16 = col.try_get_color16(); col16.has_value())
      return col16.value();
    
    if (auto rgb6 = col.try_get_rgb6(); rgb6.has_value())
    {
      auto [r, g, b] = rgb6.value().to_xterm_float();
      
      struct rgb_t { float r, g, b; };
      
      static constexpr rgb_t ansi16_table[16] =
      {
        { 0.0f, 0.0f, 0.0f },   // Black
        { 0.4f, 0.0f, 0.0f },   // DarkRed
        { 0.0f, 0.4f, 0.0f },   // DarkGreen
        { 0.4f, 0.4f, 0.0f },   // DarkYellow
        { 0.0f, 0.0f, 0.4f },   // DarkBlue
        { 0.4f, 0.0f, 0.4f },   // DarkMagenta
        { 0.0f, 0.4f, 0.4f },   // DarkCyan
        { 0.8f, 0.8f, 0.8f },   // LightGray
        { 0.4f, 0.4f, 0.4f },   // DarkGray
        { 1.0f, 0.0f, 0.0f },   // Red
        { 0.0f, 1.0f, 0.0f },   // Green
        { 1.0f, 1.0f, 0.0f },   // Yellow
        { 0.0f, 0.0f, 1.0f },   // Blue
        { 1.0f, 0.0f, 1.0f },   // Magenta
        { 0.0f, 1.0f, 1.0f },   // Cyan
        { 1.0f, 1.0f, 1.0f },   // White
      };
      
      auto min_dist_sq = math::get_max<float>();
      int best_color = 0;
      for (int color = 0; color < 16; ++color)
      {
        auto rgb2 = ansi16_table[color];
        auto dist_sq = math::distance_squared(r, g, b, rgb2.r, rgb2.g, rgb2.b);
        if (math::minimize(min_dist_sq, dist_sq))
          best_color = color;
      }
      return static_cast<Color16>(best_color + 1);
    }
    
    // 24 shades of gray.
    int idx = col.get_index();
    if (232 <= idx && idx <= 255)
    {
      int gray = idx - 232; // 0..23
      if (gray < 6)  return Color16::Black;
      if (gray < 12) return Color16::DarkGray;
      if (gray < 18) return Color16::LightGray;
      return Color16::White;
    }
    
    // Safety fallback
    return Color16::Default;
  }


}
