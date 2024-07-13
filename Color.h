//
//  Color.h
//  Termin8or
//
//  Created by Rasmus Anthin on 2024-06-20.
//

#pragma once
#include <Core/Rand.h>

enum class Color
{
  Transparent = -1,
  Transparent2 = -2,
  Default = 0,
  Black,
  DarkRed,
  DarkGreen,
  DarkYellow,
  DarkBlue,
  DarkMagenta,
  DarkCyan,
  LightGray,
  DarkGray,
  Red,
  Green,
  Yellow,
  Blue,
  Magenta,
  Cyan,
  White
};

namespace color
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

}
