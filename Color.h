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
  
  enum class ShadeType { Bright, Dark };
  // #NOTE: Returns Color::Default if no matching color was found.
  // If inputting a bright color and ShadeType is Bright, then you will get the same color.
  // If inputting a dark color and ShadeType is Bright, then you will get the corresponding bright color.
  // If inputting a bright color and ShadeType is Dark, then you will get the corresponding dark color.
  // If inputting a dark color and ShadeType is Dark, then you will get the same color.
  Color shade_color(Color color, ShadeType shade)
  {
    int idx = -1;
    switch (shade)
    {
      case ShadeType::Bright:
        if (stlutils::contains(colors_bright, color))
          return color;
        idx = stlutils::find_idx(colors_dark, color);
        if (0 <= idx)
          return colors_bright[idx];
        break;
      case ShadeType::Dark:
        if (stlutils::contains(colors_dark, color))
          return color;
        idx = stlutils::find_idx(colors_bright, color);
        if (0 <= idx)
          return colors_dark[idx];
        break;
    }
    return Color::Default;
  }

}
