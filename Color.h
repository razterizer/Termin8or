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
  enum class Color
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
}

namespace t8::color
{

  Color string2color(const std::string& str)
  {
    if (str == "Transparent")
      return Color::Transparent;
    if (str == "Transparent2")
      return Color::Transparent2;
    if (str == "Default")
      return Color::Default;
    if (str == "Black")
      return Color::Black;
    if (str == "DarkRed")
      return Color::DarkRed;
    if (str == "DarkGreen")
      return Color::DarkGreen;
    if (str == "DarkYellow")
      return Color::DarkYellow;
    if (str == "DarkBlue")
      return Color::DarkBlue;
    if (str == "DarkMagenta")
      return Color::DarkMagenta;
    if (str == "DarkCyan")
      return Color::DarkCyan;
    if (str == "LightGray")
      return Color::LightGray;
    if (str == "DarkGray")
      return Color::DarkGray;
    if (str == "Red")
      return Color::Red;
    if (str == "Green")
      return Color::Green;
    if (str == "Yellow")
      return Color::Yellow;
    if (str == "Blue")
      return Color::Blue;
    if (str == "Magenta")
      return Color::Magenta;
    if (str == "Cyan")
      return Color::Cyan;
    if (str == "White")
      return Color::White;
      
    return Color::Default;
  }
  
  std::string color2string(Color color)
  {
    switch (color)
    {
      case Color::Transparent:
        return "Transparent";
      case Color::Transparent2:
        return "Transparent2";
      case Color::Default:
        return "Default";
      case Color::Black:
        return "Black";
      case Color::DarkRed:
        return "DarkRed";
      case Color::DarkGreen:
        return "DarkGreen";
      case Color::DarkYellow:
        return "DarkYellow";
      case Color::DarkBlue:
        return "DarkBlue";
      case Color::DarkMagenta:
        return "DarkMagenta";
      case Color::DarkCyan:
        return "DarkCyan";
      case Color::LightGray:
        return "LightGray";
      case Color::DarkGray:
        return "DarkGray";
      case Color::Red:
        return "Red";
      case Color::Green:
        return "Green";
      case Color::Yellow:
        return "Yellow";
      case Color::Blue:
        return "Blue";
      case Color::Magenta:
        return "Magenta";
      case Color::Cyan:
        return "Cyan";
      case Color::White:
        return "White";
      default:
        return "";
    }
  }
  
  Color get_random_color(const std::vector<Color>& palette)
  {
    auto num = static_cast<int>(palette.size());
    if (num == 0)
      return Color::Default;
    auto idx = rnd::rand_int(0, num - 1);
    return palette[idx];
  }
  
  std::vector<Color> colors_hue_light
  {
    Color::Red,
    Color::Yellow,
    Color::Green,
    Color::Cyan,
    Color::Blue,
  };
  
  std::vector<Color> colors_hue_light_dark
  {
    Color::Red,
    Color::Yellow,
    Color::Green,
    Color::Cyan,
    Color::Blue,
    Color::Magenta,
    Color::DarkRed,
    Color::DarkYellow,
    Color::DarkGreen,
    Color::DarkCyan,
    Color::DarkBlue,
    Color::DarkMagenta,
  };
  
  // #NOTE: these color vectors are to be compared with each other, line by line.
  // These are not absolute categories! E.g. In colors_dark there is LightGray,
  //   but it is only there because it is darker than the corresponding White color
  //   in the colors_bright vector.
  std::vector<Color> colors_bright
  {
    Color::White,
    Color::LightGray,
    Color::DarkGray,
    Color::Cyan,
    Color::Magenta,
    Color::Yellow,
    Color::Red,
    Color::Green,
    Color::Blue,
  };
  std::vector<Color> colors_dark
  {
    Color::LightGray,
    Color::DarkGray,
    Color::Black,
    Color::DarkCyan,
    Color::DarkMagenta,
    Color::DarkYellow,
    Color::DarkRed,
    Color::DarkGreen,
    Color::DarkBlue,
  };
  
  enum class ShadeType { Unchanged, Bright, Dark };
  // #NOTE: Returns Color::Default if no matching color was found.
  //   If shade = ShadeType::Unchanged, then it will return the same color as the argument.
  // If inputting a bright color and ShadeType is Bright, then you will get the same color.
  // If inputting a dark color and ShadeType is Bright, then you will get the corresponding bright color.
  // If inputting a bright color and ShadeType is Dark, then you will get the corresponding dark color.
  // If inputting a dark color and ShadeType is Dark, then you will get the same color.
  Color shade_color(Color color, ShadeType shade)
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
    return Color::Default;
  }
  
  std::optional<bool> is_bright(Color color, bool perceived = false)
  {
    if (perceived)
    {
#ifndef _WIN32
      switch (color)
      {
        case Color::DarkGreen: return true;
        case Color::DarkYellow: return true;
        case Color::DarkCyan: return true;
        case Color::Red: return false;
        case Color::Blue: return false;
        default:
          break;
      }
#endif
    }
    switch (color)
    {
      case Color::Transparent:
      case Color::Transparent2:
      case Color::Default:
        return std::nullopt;
      case Color::Black:
      case Color::DarkRed:
      case Color::DarkGreen:
      case Color::DarkYellow:
      case Color::DarkBlue:
      case Color::DarkMagenta:
      case Color::DarkCyan:
        return false;
      case Color::LightGray: return true;
      case Color::DarkGray: return false;
      case Color::Red:
      case Color::Green:
      case Color::Yellow:
      case Color::Blue:
      case Color::Magenta:
      case Color::Cyan:
      case Color::White:
        return true;
    }
    assert(false && "Unhandled Color in is_bright()");
    return std::nullopt;
  }
  
  std::optional<bool> is_dark(Color color, bool perceived = false)
  {
    if (perceived)
    {
#ifndef _WIN32
      switch (color)
      {
        case Color::DarkGreen: return false;
        case Color::DarkYellow: return false;
        case Color::DarkCyan: return false;
        case Color::Red: return true;
        case Color::Blue: return true;
        default:
          break;
      }
#endif
    }
    switch (color)
    {
      case Color::Transparent:
      case Color::Transparent2:
      case Color::Default:
        return std::nullopt;
      case Color::Black:
      case Color::DarkRed:
      case Color::DarkGreen:
      case Color::DarkYellow:
      case Color::DarkBlue:
      case Color::DarkMagenta:
      case Color::DarkCyan:
        return true;
      case Color::LightGray: return false;
      case Color::DarkGray: return true;
      case Color::Red:
      case Color::Green:
      case Color::Yellow:
      case Color::Blue:
      case Color::Magenta:
      case Color::Cyan:
      case Color::White:
        return false;
    }
    assert(false && "Unhandled Color in is_dark()");
    return std::nullopt;
  }
  
  Color get_contrast_color(Color color)
  {
    switch (color)
    {
      case Color::Transparent: return Color::Transparent;
      case Color::Transparent2: return Color::Transparent2;
      case Color::Default: return Color::Default;
      case Color::Black: return Color::White;
      case Color::DarkRed: return Color::Cyan;
      case Color::DarkGreen: return Color::Magenta;
      case Color::DarkYellow: return Color::Blue;
      case Color::DarkBlue: return Color::Yellow;
      case Color::DarkMagenta: return Color::Green;
      case Color::DarkCyan: return Color::Red;
      case Color::LightGray: return Color::Black;
      case Color::DarkGray: return Color::White;
      case Color::Red: return Color::DarkCyan;
      case Color::Green: return Color::DarkMagenta;
      case Color::Yellow: return Color::DarkBlue;
      case Color::Blue: return Color::DarkYellow;
      case Color::Magenta: return Color::DarkGreen;
      case Color::Cyan: return Color::DarkRed;
      case Color::White: return Color::Black;
    }
    assert(false && "Unhandled Color in get_contrast_color()");
    return Color::Default;
  }
  
  int get_color_value_win(Color color)
  {
    switch (color)
    {
      case Color::DarkRed:
        return 4;
      case Color::DarkGreen:
        return 2;
      case Color::DarkYellow:
        return 6;
      case Color::DarkBlue:
        return 1;
      case Color::DarkMagenta:
        return 5;
      case Color::DarkCyan:
        return 3;
      case Color::LightGray:
        return 7;
      case Color::DarkGray:
        return 8;
      case Color::Red:
        return 12;
      case Color::Green:
        return 10;
      case Color::Yellow:
        return 14;
      case Color::Blue:
        return 9;
      case Color::Magenta:
        return 13;
      case Color::Cyan:
        return 11;
      case Color::White:
        return 15;
      case Color::Black:
      default:
        return 0;
    }
  }
  
  Color get_color_win(int color_val)
  {
    switch (color_val)
    {
      case 0:
        return Color::Black;
      case 1:
        return Color::DarkBlue;
      case 2:
        return Color::DarkGreen;
      case 3:
        return Color::DarkCyan;
      case 4:
        return Color::DarkRed;
      case 5:
        return Color::DarkMagenta;
      case 6:
        return Color::DarkYellow;
      case 7:
        return Color::LightGray;
      case 8:
        return Color::DarkGray;
      case 9:
        return Color::Blue;
      case 10:
        return Color::Green;
      case 11:
        return Color::Cyan;
      case 12:
        return Color::Red;
      case 13:
        return Color::Magenta;
      case 14:
        return Color::Yellow;
      case 15:
        return Color::White;
      default:
        return Color::Default;
    }
  }

}
