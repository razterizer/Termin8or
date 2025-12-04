//
//  Color.h
//  Termin8or
//
//  Created by Rasmus Anthin on 2024-06-20.
//

#pragma once
#include "RGBA.h"
#include <Core/Rand.h>
#include <Core/StringHelper.h>
#include <string>
#include <map>

namespace t8
{
  static constexpr int c_min_color_idx = -3;
  static constexpr int c_max_color_idx = 255;

  enum class Color16
  {
    Transparent2 = -3,
    Transparent = -2,
    Default = -1,
    Black,         //  0
    DarkRed,       //  1
    DarkGreen,     //  2
    DarkYellow,    //  3
    DarkBlue,      //  4
    DarkMagenta,   //  5
    DarkCyan,      //  6
    LightGray,     //  7
    DarkGray,      //  8
    Red,           //  9
    Green,         // 10
    Yellow,        // 11
    Blue,          // 12
    Magenta,       // 13
    Cyan,          // 14
    White          // 15
  };

  Color16 string_to_color16(std::string_view str, int* start_idx = nullptr)
  {
    auto f_match_str = [&str, start_idx](std::string_view needle)
    {
      auto needle_size = needle.size();
      if (start_idx == nullptr)
        return str == needle;
      if (str.size() < needle_size + *start_idx)
        return false;
      if (str.compare(*start_idx, needle_size, needle) == 0)
      {
        *start_idx += static_cast<int>(needle_size);
        return true;
      }
      return false;
    };
    
    if (f_match_str("Transparent2")) // Must come before Transparent because of suffix.
      return Color16::Transparent2;
    if (f_match_str("Transparent"))
      return Color16::Transparent;
    if (f_match_str("Default"))
      return Color16::Default;
    if (f_match_str("Black"))
      return Color16::Black;
    if (f_match_str("DarkRed"))
      return Color16::DarkRed;
    if (f_match_str("DarkGreen"))
      return Color16::DarkGreen;
    if (f_match_str("DarkYellow"))
      return Color16::DarkYellow;
    if (f_match_str("DarkBlue"))
      return Color16::DarkBlue;
    if (f_match_str("DarkMagenta"))
      return Color16::DarkMagenta;
    if (f_match_str("DarkCyan"))
      return Color16::DarkCyan;
    if (f_match_str("LightGray"))
      return Color16::LightGray;
    if (f_match_str("DarkGray"))
      return Color16::DarkGray;
    if (f_match_str("Red"))
      return Color16::Red;
    if (f_match_str("Green"))
      return Color16::Green;
    if (f_match_str("Yellow"))
      return Color16::Yellow;
    if (f_match_str("Blue"))
      return Color16::Blue;
    if (f_match_str("Magenta"))
      return Color16::Magenta;
    if (f_match_str("Cyan"))
      return Color16::Cyan;
    if (f_match_str("White"))
      return Color16::White;
      
    return Color16::Default;
  }
  
  std::string color16_to_string(Color16 color)
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
  
  int get_color16_value_win(Color16 color)
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
  
  Color16 get_color16_win(int color_val)
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
      : idx(static_cast<int>(col16))
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
      set_gray(gray24.gray);
    }
    
    int get_index() const { return idx; }
    
    constexpr bool is_color16_special() const noexcept
    {
      return -3 <= idx && idx <= -1;
    }
    
    constexpr bool is_color16_regular() const noexcept
    {
      return 0 <= idx && idx <= 15;
    }
    
    constexpr bool is_color16() const noexcept
    {
      return -3 <= idx && idx <= 15;
    }
    
    constexpr bool is_rgb6() const noexcept
    {
      return 16 <= idx && idx <= 231;
    }
    
    constexpr bool is_gray24() const noexcept
    {
      return 232 <= idx && idx <= 255;
    }
    
    std::optional<Color16> try_get_color16() const
    {
      if (is_color16())
        return static_cast<Color16>(idx);
      return std::nullopt;
    }

    std::optional<RGB6> try_get_rgb6() const
    {
      // Do not confuse with Color16::White = 16!
      // For idx = 0 .. 15, this corresponds to Color16::Black to Color16::White.
      if (is_rgb6())
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
      if (is_gray24())
      {
        int v = idx - 232;
        return static_cast<uint8_t>(v);
      }
      return std::nullopt;
    }
    
    inline bool operator==(Color other) const { return this->idx == other.idx; }
    inline bool operator==(Color16 col16) const { return this->idx == static_cast<int>(col16); }
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
    
    inline bool operator<(Color other) const
    {
      return this->idx < other.idx;
    }
    
    std::string str(bool compact = false) const
    {
      if (auto col16 = try_get_color16(); col16.has_value())
      {
        if (!compact)
          return color16_to_string(col16.value());
        
        switch (col16.value())
        {
          case Color16::Transparent:  return "t";
          case Color16::Transparent2: return "T";
          case Color16::Default:      return "*";
          case Color16::Black:        return "0";
          case Color16::DarkRed:      return "1";
          case Color16::DarkGreen:    return "2";
          case Color16::DarkYellow:   return "3";
          case Color16::DarkBlue:     return "4";
          case Color16::DarkMagenta:  return "5";
          case Color16::DarkCyan:     return "6";
          case Color16::LightGray:    return "7";
          case Color16::DarkGray:     return "8";
          case Color16::Red:          return "9";
          case Color16::Green:        return "A";
          case Color16::Yellow:       return "B";
          case Color16::Blue:         return "C";
          case Color16::Magenta:      return "D";
          case Color16::Cyan:         return "E";
          case Color16::White:        return "F";
          default: return "*";
        }
      }
      
      if (auto rgb6 = try_get_rgb6(); rgb6.has_value())
      {
        std::string ret = compact ? "[" : "rgb6:[";
        ret += std::to_string(rgb6.value().r) + (compact ? "" : ", ");
        ret += std::to_string(rgb6.value().g) + (compact ? "" : ", ");
        ret += std::to_string(rgb6.value().b) + "]";
        return ret;
      }
      
      if (auto g24 = try_get_gray24(); g24.has_value())
      {
        int gray = g24.value().gray;
        std::string gray_str = compact ? str::int2hex(gray) : std::to_string(gray);
        std::string ret = compact ? "{" : "gray24:{";
        ret += gray_str + "}";
        return ret;
      }
      
      return compact ? "*" : "n/a";
    }
    
    bool parse(const std::string& str, int& start_idx, bool compact = false)
    {
      if (start_idx >= str.size())
        return false;
    
      std::string_view remaining(str);
      remaining.remove_prefix(start_idx);
    
      static const char* rgb6_prefix = "rgb6:";
      if (!remaining.empty() && (remaining[0] == '[' || remaining.starts_with(rgb6_prefix)))
      {
        if (auto i = remaining.find(']'); i == std::string::npos)
        {
          std::cerr << "ERROR in Color::parse() : Unable to parse ']' token after rgb6 triplet!\n";
          return false;
        }
        else
        {
          if (remaining.starts_with(rgb6_prefix))
            start_idx += std::strlen(rgb6_prefix);
          int end_idx = start_idx + static_cast<int>(i);
          auto substr = str.substr(start_idx, end_idx - start_idx + 1);
          auto rgb6_tokens = str::tokenize(substr, { '[', ']', ',', ' ' });
          if (rgb6_tokens.size() == 1 && rgb6_tokens[0].size() == 3)
          {
            int r = str::to_digit(rgb6_tokens[0][0]);
            int g = str::to_digit(rgb6_tokens[0][1]);
            int b = str::to_digit(rgb6_tokens[0][2]);
            if (r < 0 || g < 0 || b < 0)
            {
              std::cerr << "ERROR in Color::parse() : Unable to parse compact rgb triplet!\n";
              return false;
            }
            start_idx = end_idx + 1;
            set_rgb(r, g, b);
            return true;
          }
          if (rgb6_tokens.size() == 3 && rgb6_tokens[0].size() == 1)
          {
            int r = str::to_digit(rgb6_tokens[0][0]);
            int g = str::to_digit(rgb6_tokens[1][0]);
            int b = str::to_digit(rgb6_tokens[2][0]);
            if (r < 0 || g < 0 || b < 0)
            {
              std::cerr << "ERROR in Color::parse() : Unable to parse non-compact rgb triplet!\n";
              return false;
            }
            start_idx = end_idx + 1;
            set_rgb(r, g, b);
            return true;
          }
        }
        std::cout << "ERROR in Color::parse() : Unable to parse rgb6 color in [%i%i%i] substring!\n";
        return false;
      }
      
      static const char* gray24_prefix = "gray24:";
      if (!remaining.empty() && (remaining[0] == '{' || remaining.starts_with(gray24_prefix)))
      {
        if (auto i = remaining.find('}'); i == std::string::npos)
        {
          std::cerr << "ERROR in Color::parse() : Unable to parse ']' token after rgb6 triplet!\n";
          return false;
        }
        else
        {
          auto compact = !remaining.starts_with(gray24_prefix);
          if (!compact)
            start_idx += std::strlen(gray24_prefix);
          int end_idx = start_idx + static_cast<int>(i);
          auto substr = str.substr(start_idx, end_idx - start_idx + 1);
          auto gr24_tokens = str::tokenize(substr, { '{', '}' });
          if (gr24_tokens.size() == 1)
          {
            auto token = gr24_tokens[0];
            int gr24 = compact ? str::hex2int(token) : std::stoi(token);
            start_idx = end_idx + 1;
            set_gray(gr24);
            return true;
          }
        }
        std::cout << "ERROR in Color::parse() : Unable to parse gray24 color in {%i} substring!\n";
        return false;
      }
      
      if (remaining.starts_with("Color16::"))
      {
        auto col16 = string_to_color16(remaining, &start_idx);
        set_color16(col16);
        return true;
      }
      {
        auto start_idx_old = start_idx;
        auto col16 = string_to_color16(remaining, &start_idx);
        if (start_idx > start_idx_old)
        {
          set_color16(col16);
          return true;
        }
      }
      if (!str.empty())
      {
        char ch = str[start_idx++];
        switch (ch)
        {
          case 't': set_color16(Color16::Transparent); break;
          case 'T': set_color16(Color16::Transparent2); break;
          case '*': set_color16(Color16::Default); break;
          case '0': set_color16(Color16::Black); break;
          case '1': set_color16(Color16::DarkRed); break;
          case '2': set_color16(Color16::DarkGreen); break;
          case '3': set_color16(Color16::DarkYellow); break;
          case '4': set_color16(Color16::DarkBlue); break;
          case '5': set_color16(Color16::DarkMagenta); break;
          case '6': set_color16(Color16::DarkCyan); break;
          case '7': set_color16(Color16::LightGray); break;
          case '8': set_color16(Color16::DarkGray); break;
          case '9': set_color16(Color16::Red); break;
          case 'A': set_color16(Color16::Green); break;
          case 'B': set_color16(Color16::Yellow); break;
          case 'C': set_color16(Color16::Blue); break;
          case 'D': set_color16(Color16::Magenta); break;
          case 'E': set_color16(Color16::Cyan); break;
          case 'F': set_color16(Color16::White); break;
          default: set_color16(Color16::Default); return false;
        }
        return true;
      }
      return false;
    }
    
    bool parse(const std::string& str, bool compact = false)
    {
      int start_idx = 0;
      return parse(str, start_idx, compact);
    }
    
  private:
    int idx = 0;
    
    void set_color16(Color16 col16)
    {
      idx = static_cast<int>(col16);
    }
    
    void set_rgb(int r, int g, int b)
    {
      idx = 16 + 36*r + 6*g + b;
    }
    
    void set_gray(int gray)
    {
      idx = 232 + gray;
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
      int best_color = 0; // Black.
      for (int color = 0; color < 16; ++color)
      {
        auto rgb2 = ansi16_table[color];
        auto dist_sq = math::distance_squared(r, g, b, rgb2.r, rgb2.g, rgb2.b);
        if (math::minimize(min_dist_sq, dist_sq))
          best_color = color;
      }
      return static_cast<Color16>(best_color);
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
    
    // Safety fallback.
    return Color16::Default;
  }
  
  static std::map<Color, RGBA> color2rgba = []()
  {
    auto colmap = std::map<Color, RGBA>
    {
      { Color16::White, RGBA { 1, 1, 1 } },
      { Color16::Cyan, RGBA { 0, 1, 1 } },
      { Color16::Magenta, RGBA { 1, 0, 1 } },
      { Color16::Blue, RGBA { 0, 0, 1 } },
      { Color16::Yellow, RGBA { 1, 1, 0 } },
      { Color16::Green, RGBA { 0, 1, 0 } },
      { Color16::Red, RGBA { 1, 0, 0 } },
      { Color16::LightGray, RGBA { 0.8, 0.8, 0.8 } },
      { Color16::DarkGray, RGBA { 0.4, 0.4, 0.4 } },
      { Color16::DarkCyan, RGBA { 0, 0.4, 0.4 } },
      { Color16::DarkMagenta, RGBA { 0.4, 0, 0.4 } },
      { Color16::DarkBlue, RGBA { 0, 0, 0.4 } },
      { Color16::DarkYellow, RGBA { 0.4, 0.4, 0 } },
      { Color16::DarkGreen, RGBA { 0, 0.4, 0 } },
      { Color16::DarkRed, RGBA { 0.4, 0, 0 } },
      { Color16::Black, RGBA { 0, 0, 0 } },
      { Color16::Transparent2, RGBA { 0, 0, 0, 0 } },
    };
    
    const int steps[6] = {0, 95, 135, 175, 215, 255};
    
    for (int r = 0; r < 6; r++)
      for (int g = 0; g < 6; g++)
        for (int b = 0; b < 6; b++)
        {
          int idx = 16 + 36*r + 6*g + b;
          colmap[Color(idx)] = RGBA
          {
            steps[r] / 255.0,
            steps[g] / 255.0,
            steps[b] / 255.0
          };
        }
        
    for (int i = 232; i <= 255; i++)
    {
      int v = 8 + (i - 232) * 10;     // 8, 18, 28, ..., 238
      double f = v / 255.0;
      colmap[Color(i)] = RGBA {f, f, f};
    }
    
    return colmap;
  }();
  
  std::optional<bool> is_bright(Color color, bool perceived_color16 = false)
  {
    if (perceived_color16)
    {
#ifndef _WIN32
      if (color.is_color16())
      {
        switch (color.try_get_color16().value())
        {
          case Color16::DarkGreen: return true;
          case Color16::DarkYellow: return true;
          case Color16::DarkCyan: return true;
          case Color16::Red: return false;
          case Color16::Blue: return false;
          default:
            break;
        }
      }
#endif
    }
    if (color.is_color16())
    {
      switch (color.try_get_color16().value())
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
    }
    
    if (auto it = color2rgba.find(color); it != color2rgba.end())
    {
      const auto& rgba = it->second;
      
      // Transparent shading: treat alpha=0 as undefined.
      if (rgba.a == 0)
        return std::nullopt;
      
      double L = luminance(rgba);
      
      // Choose threshold. Typical UI threshold ≈ 0.5.
      return L >= 0.5;
    }
    
    assert(false && "Unhandled Color in is_bright()");
    return std::nullopt;
  }
  
  std::optional<bool> is_dark(Color color, bool perceived_color16 = false)
  {
    if (perceived_color16)
    {
#ifndef _WIN32
      if (color.is_color16())
      {
        switch (color.try_get_color16().value())
        {
          case Color16::DarkGreen: return false;
          case Color16::DarkYellow: return false;
          case Color16::DarkCyan: return false;
          case Color16::Red: return true;
          case Color16::Blue: return true;
          default:
            break;
        }
      }
#endif
    }
    if (color.is_color16())
    {
      switch (color.try_get_color16().value())
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
    }
    
    if (auto it = color2rgba.find(color); it != color2rgba.end())
    {
      const auto& rgba = it->second;
      
      // Transparent shading: treat alpha=0 as undefined.
      if (rgba.a == 0)
        return std::nullopt;
      
      double L = luminance(rgba);
      
      // Choose threshold. Typical UI threshold ≈ 0.5.
      return L < 0.5;
    }
    
    assert(false && "Unhandled Color in is_dark()");
    return std::nullopt;
  }
  
  Color get_contrast_color(Color color)
  {
    if (color.is_color16())
    {
      switch (color.try_get_color16().value())
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
    }
    
    if (auto rgb6 = color.try_get_rgb6(); rgb6.has_value())
    {
      auto [r, g, b] = rgb6.value();
      return RGB6(5 - r, 5 - g, 5 - b);
    }
    
    if (auto gray24 = color.try_get_gray24(); gray24.has_value())
      return Gray24(23 - gray24.value().gray);
    
    assert(false && "Unhandled Color in get_contrast_color()");
    return Color16::Default;
  }


}
