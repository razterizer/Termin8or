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
    char32_t preferred = U'?';
    char fallback = U'?';
    
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
