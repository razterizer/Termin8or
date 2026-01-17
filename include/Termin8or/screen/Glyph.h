//
//  Glyph.h
//  Termin8or
//
//  Created by Rasmus Anthin on 2026-01-07.
//

#pragma once

namespace t8
{
  
  struct Glyph
  {
    static const char32_t none32 = 0xFFFFFFFFu; // char32_t is unsigned.
    static const char none = '\0'; // NUL character. No use for such characeters in this context.
    
    char32_t preferred = none32;
    char fallback = none;
    
    Glyph() = default;
    
    Glyph(char32_t pref, char fb = U'?')
      : preferred(pref)
      , fallback(fb)
    {}
    
    bool operator==(const Glyph& g) const
    {
      return this->preferred == g.preferred
          && this->fallback == g.fallback;
    }
  };
  
}
