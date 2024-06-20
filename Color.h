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

}
