//
//  Rectangle_tests.h
//  Termin8or
//
//  Created by Rasmus Anthin on 2024-09-22.
//

#pragma once
#include "../Rectangle.h"
#include <iostream>
#include <cassert>

namespace ttl
{

  void unit_tests()
  {
    RC pt0 { 0, 0 };
    RC pt1 { 1, 0 };
    RC pt2 { 0, 1 };
    RC pt3 { -1, 0 };
    RC pt4 { 0, -1 };
  
    {
      Rectangle rec;
      
      assert(rec.r == 0);
      assert(rec.c == 0);
      assert(rec.r_len == 0);
      assert(rec.c_len == 0);
      
      assert(!rec.is_inside(0, 0));
      assert(!rec.is_inside(1, 0));
      assert(!rec.is_inside(0, 1));
      assert(!rec.is_inside(-1, 0));
      assert(!rec.is_inside(0, -1));
      assert(!rec.is_inside(pt0));
      assert(!rec.is_inside(pt1));
      assert(!rec.is_inside(pt2));
      assert(!rec.is_inside(pt3));
      assert(!rec.is_inside(pt4));
      
      assert(!rec.is_inside_offs(0, 0, -1));
      assert(!rec.is_inside_offs(1, 0, -1));
      assert(!rec.is_inside_offs(0, 1, -1));
      assert(!rec.is_inside_offs(-1, 0, -1));
      assert(!rec.is_inside_offs(0, -1, -1));
      assert(!rec.is_inside_offs(pt0, -1));
      assert(!rec.is_inside_offs(pt1, -1));
      assert(!rec.is_inside_offs(pt2, -1));
      assert(!rec.is_inside_offs(pt3, -1));
      assert(!rec.is_inside_offs(pt4, -1));
      
      assert(rec.is_inside_offs(0, 0, +1));
      assert(!rec.is_inside_offs(1, 0, +1));
      assert(!rec.is_inside_offs(0, 1, +1));
      assert(rec.is_inside_offs(-1, 0, +1));
      assert(rec.is_inside_offs(0, -1, +1));
      assert(rec.is_inside_offs(pt0, +1));
      assert(!rec.is_inside_offs(pt1, +1));
      assert(!rec.is_inside_offs(pt2, +1));
      assert(rec.is_inside_offs(pt3, +1));
      assert(rec.is_inside_offs(pt4, +1));
      
      assert(rec.is_inside_offs(0, 0, +2));
      assert(rec.is_inside_offs(1, 0, +2));
      assert(rec.is_inside_offs(0, 1, +2));
      assert(rec.is_inside_offs(-1, 0, +2));
      assert(rec.is_inside_offs(0, -1, +2));
      assert(rec.is_inside_offs(pt0, +2));
      assert(rec.is_inside_offs(pt1, +2));
      assert(rec.is_inside_offs(pt2, +2));
      assert(rec.is_inside_offs(pt3, +2));
      assert(rec.is_inside_offs(pt4, +2));
      
      assert(rec.is_inside_offs(pt0, +1, +2));
      assert(!rec.is_inside_offs(pt1, +1, +2));
      assert(rec.is_inside_offs(pt2, +1, +2));
      assert(rec.is_inside_offs(pt3, +1, +2));
      assert(rec.is_inside_offs(pt4, +1, +2));
      
      assert(rec.is_inside_offs(pt0, +1, +2, 0, +1));
      assert(!rec.is_inside_offs(pt0, +1, +2, +1, 0));
      assert(!rec.is_inside_offs(pt0, +2, 0, 0, +1));
      assert(rec.is_inside_offs(pt0, 0, +1, +1, +1));
      assert(!rec.is_inside_offs(pt0, 0, +1, -1, +1));
      assert(!rec.is_inside_offs(pt0, -1, +1, -1, +1));
      assert(!rec.is_inside_offs(pt0, +1, -1, +1, -1));
      assert(rec.is_inside_offs(pt0, +1, +1, +1, +1));
      assert(rec.is_inside_offs(pt0, +2, +2, +2, +2));
      
      assert(rec.is_inside_offs(pt1, +1, +2, 0, +1));
      assert(!rec.is_inside_offs(pt1, +1, +2, +1, 0));
      assert(!rec.is_inside_offs(pt1, +2, 0, 0, +1));
      assert(!rec.is_inside_offs(pt1, 0, +1, +1, +1));
      assert(!rec.is_inside_offs(pt1, 0, +1, -1, +1));
      assert(!rec.is_inside_offs(pt1, -1, +1, -1, +1));
      assert(!rec.is_inside_offs(pt1, +1, -1, +1, -1));
      assert(!rec.is_inside_offs(pt1, +1, +1, +1, +1));
      assert(rec.is_inside_offs(pt1, +2, +2, +2, +2));
      
      assert(!rec.is_inside_offs(pt2, +1, +2, 0, +1));
      assert(!rec.is_inside_offs(pt2, +1, +2, +1, 0));
      assert(!rec.is_inside_offs(pt2, +2, 0, 0, +1));
      assert(!rec.is_inside_offs(pt2, 0, +1, +1, +1));
      assert(!rec.is_inside_offs(pt2, 0, +1, -1, +1));
      assert(!rec.is_inside_offs(pt2, -1, +1, -1, +1));
      assert(!rec.is_inside_offs(pt2, +1, -1, +1, -1));
      assert(!rec.is_inside_offs(pt2, +1, +1, +1, +1));
      assert(rec.is_inside_offs(pt2, +2, +2, +2, +2));
      
      assert(rec.is_inside_offs(pt3, +1, +2, 0, +1));
      assert(!rec.is_inside_offs(pt3, +1, +2, +1, 0));
      assert(rec.is_inside_offs(pt3, +2, 0, 0, +1));
      assert(!rec.is_inside_offs(pt3, 0, +1, +1, +1));
      assert(!rec.is_inside_offs(pt3, 0, +1, -1, +1));
      assert(!rec.is_inside_offs(pt3, -1, +1, -1, +1));
      assert(!rec.is_inside_offs(pt3, +1, -1, +1, -1));
      assert(rec.is_inside_offs(pt3, +1, +1, +1, +1));
      assert(rec.is_inside_offs(pt3, +2, +2, +2, +2));
      
      assert(!rec.is_inside_offs(pt4, +1, +2, 0, +1));
      assert(rec.is_inside_offs(pt4, +1, +2, +1, 0));
      assert(!rec.is_inside_offs(pt4, +2, 0, 0, +1));
      assert(rec.is_inside_offs(pt4, 0, +1, +1, +1));
      assert(!rec.is_inside_offs(pt4, 0, +1, -1, +1));
      assert(!rec.is_inside_offs(pt4, -1, +1, -1, +1));
      assert(!rec.is_inside_offs(pt4, +1, -1, +1, -1));
      assert(rec.is_inside_offs(pt4, +1, +1, +1, +1));
      assert(rec.is_inside_offs(pt4, +2, +2, +2, +2));
    }
  }
  
}
