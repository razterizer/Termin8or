//
//  Glyph_tests.h
//  Termin8or
//
//  Created by Rasmus Anthin on 2026-01-13.
//

#pragma once
#include "screen/Glyph.h"
#include <iostream>
#include <cassert>

namespace glyph
{

  void unit_tests()
  {
    using namespace t8;
  
    Glyph g0; // none32, none
    Glyph g1 = Glyph::none32; // none32, none
    Glyph g2 = 'A'; // A, none
    //Glyph g3 = 0x2603; // 0x2603, none (illegal)
    Glyph g4 { Glyph::none32, Glyph::none }; // none32, none
    //Glyph g5 { Glyph::none32, 'A' }; // none32, A (illegal)
    Glyph g6 { 'A', Glyph::none }; // A, none
    Glyph g7 { 'A', 'A' }; // A, A
    //Glyph g8 { 0x2603, Glyph::none }; // 0x2603, none (illegal)
    Glyph g9 { 0x2603, 'A' }; // 0x2603, A
    Glyph g10 { 'A', 'B' };
  
    {
      assert(g0.str(false) == "[]");
      assert(g0.str(true) == "?");
      
      assert(g1.str(false) == "[]");
      assert(g1.str(true) == "?");
      
      assert(g2.str(false) == "[A]");
      assert(g2.str(true) == "A");
            
      assert(g4.str(false) == "[]");
      assert(g4.str(true) == "?");
            
      assert(g6.str(false) == "[A]");
      assert(g6.str(true) == "A");
      
      assert(g7.str(false) == "[A]");
      assert(g7.str(true) == "A");
            
      assert(g9.str(false) == "[2603,A]");
      assert(g9.str(true) == "A");
      
      assert(g10.str(false) == "[A]");
      assert(g10.str(true) == "A");
    }
    
    {
      //assert(!g0.parse("?"));
      
      assert(g0.parse("[]", false));
      assert(g0.preferred == Glyph::none32);
      assert(g0.fallback == Glyph::none);
      
      assert(g2.parse("[A]", false));
      assert(g2.preferred == U'A');
      assert(g2.fallback == 'A');
      
      //assert(!g0.parse("[2603]", false)); // error
      //assert(g8.preferred == 0x2603);
      //assert(g8.fallback == Glyph::none);
      
      assert(g9.parse("[2603,A]", false));
      //std::cout << g9.str() << std::endl;
      assert(g9.preferred == 0x2603);
      assert(g9.fallback == 'A');
    }
    {
      Glyph g;
      bool ok = true;
      int l_idx = 0;
      for (;;)
      {
        ok = g.parse("[-][A][4][2603,8],[=]", l_idx, false, false);
        if (!ok)
        {
          assert(l_idx == 17);
          break;
        }
      }
      
      ok = true;
      l_idx = 0;
      for (;;)
      {
        ok = g.parse("[-][A][4][2603,8][=]", l_idx, false, true);
        if (!ok)
        {
          assert(l_idx == 20);
          break;
        }
      }
    }
  }
}
