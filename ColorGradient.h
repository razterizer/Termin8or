#pragma once
#include "Text.h"

struct ColorGradient
{
  std::vector<std::pair<float, Text::Color>> colors;
  
  ColorGradient() = default;
  ColorGradient(std::vector<std::pair<float, Text::Color>> cols)
    : colors(cols) { }
  
  Text::Color operator() (float t) const
  {
    assert(0.f <= t && t <= 1.f);
    float t_dist_min = 2.f;
    Text::Color closest_color = Text::Color::Default;
    for (const auto& col_pair : colors)
    {
      auto t_dist = std::abs(t - col_pair.first);
      if (t_dist < t_dist_min)
      {
        closest_color = col_pair.second;
        t_dist_min = t_dist;
      }
    }
    return closest_color;
  }
};
