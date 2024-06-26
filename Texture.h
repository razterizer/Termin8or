//
//  Texture.h
//  Termin8or
//
//  Created by Rasmus Anthin on 2024-06-25.
//

#pragma once
#include "RC.h"
#include "Color.h"


namespace drawing
{

  // RC.r = 1, RC.c = 1 yields a 1x1 texture.
  struct Texture
  {
    RC size;
    int area = 0;
    std::vector<char> characters;
    std::vector<Color> fg_colors;
    std::vector<Color> bg_colors;
    
    Texture() = default;
    Texture(const RC& tex_size)
      : size(tex_size)
      , area(tex_size.r*tex_size.c)
      , characters(area, ' ')
      , fg_colors(area, Color::Default)
      , bg_colors(area, Color::Transparent2)
    {}
    Texture(int tex_rows, int tex_cols)
      : size({ tex_rows, tex_cols })
      , area(tex_rows*tex_cols)
      , fg_colors(area, Color::Default)
      , bg_colors(area, Color::Transparent2)
    {}
    
  };

}
