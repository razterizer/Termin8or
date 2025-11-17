#pragma once
#include "Pixel.h"

namespace t8x
{

  struct LineData
  {
    std::string str;
    std::vector<Color16> fg_colors;
    std::vector<Color16> bg_colors;
    
    LineData(const std::string& s, const std::vector<Color16>& fg, const std::vector<Color16>& bg)
      : str(s), fg_colors(fg), bg_colors(bg) { }
    
    std::vector<Pixel> get_pixels() const
    {
      std::vector<Pixel> pixels;
      size_t N = std::min(str.size(), std::min(fg_colors.size(), bg_colors.size()));
      for (size_t i = 0; i < N; ++i)
        pixels.emplace_back(str[i], fg_colors[i], bg_colors[i]);
      return pixels;
    }
  };

}
