#pragma once
#include "../Core/Math.h"

// Bresenham Algorithm.
namespace bresenham
{
  template<int NR, int NC>
  void plot_line_low(SpriteHandler<NR, NC>& sh, float x0, float y0, float x1, float y1,
                     const std::string& str, Text::Color fg_color, Text::Color bg_color)
  {
    auto dx = x1 - x0;
    auto dy = y1 - y0;
    auto yi = 1.f;
    if (dy < 0.f)
    {
      yi = -1.f;
      dy = -dy;
    }
    auto D = 2*dy - dx;
    auto y = y0;
    
    for (auto x : math::linspace(x0, 1.f, x1))
    {
      sh.write_buffer(str, y, x, fg_color, bg_color);
      if (D > 0)
      {
        y += yi;
        D += 2*(dy - dx);
      }
      else
        D += 2*dy;
    }
  }

  template<int NR, int NC>
  void plot_line_high(SpriteHandler<NR, NC>& sh, float x0, float y0, float x1, float y1,
                      const std::string& str, Text::Color fg_color, Text::Color bg_color)
  {
    auto dx = x1 - x0;
    auto dy = y1 - y0;
    auto xi = 1.f;
    if (dx < 0.f)
    {
      xi = -1.f;
      dx = -dx;
    }
    auto D = 2*dx - dy;
    auto x = x0;
    
    for (auto y : math::linspace(y0, 1.f, y1))
    {
      sh.write_buffer(str, y, x, fg_color, bg_color);
      if (D > 0)
      {
        x += xi;
        D += 2*(dx - dy);
      }
      else
        D += 2*dx;
    }
  }

  template<int NR, int NC>
  void plot_line(SpriteHandler<NR, NC>& sh, float x0, float y0, float x1, float y1,
                 const std::string& str, Text::Color fg_color, Text::Color bg_color)
  {
    if (std::abs(y1 - y0) < std::abs(x1 - x0))
    {
      if (x0 > x1)
        plot_line_low(sh, x1, y1, x0, y0, str, fg_color, bg_color);
      else
        plot_line_low(sh, x0, y0, x1, y1, str, fg_color, bg_color);
    }
    else
    {
      if (y0 > y1)
        plot_line_high(sh, x1, y1, x0, y0, str, fg_color, bg_color);
      else
        plot_line_high(sh, x0, y0, x1, y1, str, fg_color, bg_color);
    }
  }
}
