//
//  Color_tests.h
//  Termin8or
//
//  Created by Rasmus Anthin on 2025-12-04.
//

#pragma once
#include "screen/Color.h"
#include <iostream>
#include <cassert>

namespace color
{

  void unit_tests()
  {
    using namespace t8;
  
    Color c16_T = Color16::Transparent2;
    Color c16_t = Color16::Transparent;
    Color c16_D = Color16::Default;
    Color c16_0 = Color16::Black;
    Color c16_F = Color16::White;
    Color crgb6_143 = RGB6(1, 4, 3);
    Color cgr24_23 = Gray24(23);
  
    {
      assert(c16_T.str(true) == "T");
      assert(c16_T.str(false) == "Transparent2");
      
      assert(c16_t.str(true) == "t");
      assert(c16_t.str(false) == "Transparent");
      
      assert(c16_D.str(true) == "*");
      assert(c16_D.str(false) == "Default");
      
      assert(c16_0.str(true) == "0");
      assert(c16_0.str(false) == "Black");
      
      assert(c16_F.str(true) == "F");
      assert(c16_F.str(false) == "White");
      
      assert(crgb6_143.str(true) == "[143]");
      assert(crgb6_143.str(false) == "rgb6:[1, 4, 3]");
      
      assert(cgr24_23.str(true) == "{17}");
      assert(cgr24_23.str(false) == "gray24:{23}");
    }
    
    {
      Color col;
      
      col.parse("T");
      assert(col.str(true) == "T");
      col.parse("Transparent2");
      assert(col.str(true) == "T");
      
      col.parse("t");
      assert(col.str(true) == "t");
      col.parse("Transparent");
      assert(col.str(true) == "t");
      
      col.parse("*");
      assert(col.str(true) == "*");
      col.parse("Default");
      assert(col.str(true) == "*");
      
      col.parse("0");
      assert(col.str(true) == "0");
      col.parse("Black");
      assert(col.str(true) == "0");
      
      col.parse("F");
      assert(col.str(true) == "F");
      col.parse("White");
      assert(col.str(true) == "F");
      
      col.parse("[143]");
      assert(col.str(true) == "[143]");
      col.parse("rgb6:[1, 4, 3]");
      assert(col.str(true) == "[143]");
      
      col.parse("{17}");
      assert(col.str(true) == "{17}");
      col.parse("gray24:{23}");
      assert(col.str(true) == "{17}");
    }
    {
      Color col;
      for (int i = 0; i < 40; ++i)
      {
        int l_idx = i;
        col.parse("****************************************", l_idx);
        assert(col.str(true) == "*");
        assert(l_idx == i + 1);
      }
    }
  }
}
