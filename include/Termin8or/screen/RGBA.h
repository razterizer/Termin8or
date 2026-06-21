//
//  RGBA.h
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
  
  inline double luminance(const RGBA& c)
  {
    return 0.2126 * c.r + 0.7152 * c.g + 0.0722 * c.b;
  }

}
