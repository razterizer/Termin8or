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
    
    Glyph(char32_t pref, char fb = none)
      : preferred(pref)
      , fallback(fb)
    {
      // 1) No fallback without preferred.
      assert(!(preferred == none32 && fallback != none)
             && "ERROR in Glyph(char32_t, char) : Fallback without preferred.");
      
      // 2) Fallback must be ASCII if present.
      assert(!(fallback != none && static_cast<unsigned char>(fallback) > 0x7F)
             && "ERROR in Glyph(char32_t, char) : Fallback must be ASCII (<=0x7F).");
      
      // 3) Preferred must be ASCII if fallback is not present.
      assert(!(preferred != none32 && preferred > 0x7F && fallback == none)
             && "ERROR in Glyph(char32_t, char) : Preferred cannot be non-ASCII while fallback is unset.");
             
      // Canonicalize ASCII glyphs.
      if (preferred != none32 && preferred <= 0x7F)
        fallback = static_cast<char>(preferred);
    }
    
    bool operator==(const Glyph& g) const
    {
      return this->preferred == g.preferred
          && this->fallback == g.fallback;
    }
  };
  
}
