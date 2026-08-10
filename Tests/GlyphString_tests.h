//
//  GlyphString_tests.h
//  Termin8or
//
//  Created by Rasmus Anthin on 2026-05-27.
//

#pragma once
#include "screen/GlyphString.h"
#include <cassert>

namespace glyph_string
{

  void unit_tests()
  {
    using namespace t8;
    using namespace t8::literals;
    
    {
      auto gs = GlyphString::from_ascii("HP");
      assert(gs.size() == 2);
      assert(gs[0] == Glyph { 'H' });
      assert(gs[1] == Glyph { 'P' });
      assert(gs.encode<char>() == "HP");
    }
    
    {
      auto gs = GlyphString::from_utf8("A\xE2\x99\xA5", 'v'); // A♥
      assert(gs.size() == 2);
      assert(gs[0] == Glyph { 'A' });
      assert((gs[1] == Glyph { U'♥', 'v' }));
      assert(gs.encode<char>() == "Av");
    }
    
    {
      auto gs = "HP: "_gs + Glyph { U'♥', 'v' } + " "_gs + GlyphString::from_number(12);
      assert(gs.size() == 8);
      assert(gs.encode<char>() == "HP: v 12");
    }
    
    {
      auto hay = GlyphString::from_ascii("abcdef");
      auto needle = GlyphString::from_ascii("cd");
      assert(hay.find(needle) == 2);
      assert(hay.substr(2, 3).encode<char>() == "cde");
      hay.clear();
      assert(hay.empty());
    }
  }
}
