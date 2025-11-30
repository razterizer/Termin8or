//
//  ColorMap.h
//  Termin8or
//
//  Created by Rasmus Anthin on 2025-11-29.
//

#pragma once

namespace t8
{

  struct RGBA
  {
    RGBA() : r(0), g(0), b(0), a(1) {}
    RGBA(double v) : r(v), g(v), b(v), a(v) {}
    RGBA(double rv, double gv, double bv) : r(rv), g(gv), b(bv), a(1) {}
    RGBA(double rv, double gv, double bv, double av) : r(rv), g(gv), b(bv), a(av) {}
    double r = 0, g = 0, b = 0, a = 0;
    RGBA operator+(const RGBA& other) { return { r+other.r, g+other.g, b+other.b, a+other.a }; }
    RGBA operator*(const RGBA& other) { return { r*other.r, g*other.g, b*other.b, a*other.a }; }
    RGBA operator*(double v) { return { r*v, g*v, b*v, a*v }; };
  };
  inline RGBA operator*(double v, const RGBA& rgba) { return { v*rgba.r, v*rgba.g, v*rgba.b, v*rgba.a }; }
  
  double clamp(double t) { return math::clamp(t, 0., 1.); }
  RGBA clamp(const RGBA& t) { return RGBA { math::clamp(t.r, 0., 1.), math::clamp(t.g, 0., 1.), math::clamp(t.b, 0., 1.), math::clamp(t.a, 0., 1.) }; }
  
  inline double luminance(const RGBA& c)
  {
    return 0.2126 * c.r + 0.7152 * c.g + 0.0722 * c.b;
  }
  
  // ////////////////////////////////////////////////

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

}
