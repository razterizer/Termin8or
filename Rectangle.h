#pragma once

namespace ttl
{
  
  struct Rectangle
  {
    Rectangle(int r, int c, int w, int h)
      : x(c), y(r), width(w), height(h)
    {}
  
    int x = 0;
    int y = 0;
    int width = 0;
    int height = 0; // #NOTE: Downwards height!
  
    bool is_inside(int r, int c)
    {
      return y <= r && r <= y + height && x <= c && c <= x + width;
    }
  };
  
}
