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
      
      rec.clear();
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
      
      assert(!rec.is_on_border(pt0));
      assert(!rec.is_on_border(pt1));
      assert(!rec.is_on_border(pt2));
      assert(!rec.is_on_border(pt3));
      assert(!rec.is_on_border(pt4));
      
      assert(rec.find_location(pt0) == BBLocation::None);
      assert(rec.find_location(pt1) == BBLocation::None);
      assert(rec.find_location(pt2) == BBLocation::None);
      assert(rec.find_location(pt3) == BBLocation::None);
      assert(rec.find_location(pt4) == BBLocation::None);
      
      assert(rec.find_location_offs(pt0, 0, +1, 0, +1) == BBLocation::Inside);
      assert(rec.find_location_offs(pt0, -1, +1, 0, +1) == BBLocation::None);
      assert(rec.find_location_offs(pt1, 0, +1, 0, +1) == BBLocation::OutsideBottom);
      assert(rec.find_location_offs(pt1, 0, +2, 0, +1) == BBLocation::Inside);
      assert(rec.find_location_offs(pt2, 0, +1, 0, +1) == BBLocation::OutsideRight);
      assert(rec.find_location_offs(pt2, 0, +1, 0, +2) == BBLocation::Inside);
      assert(rec.find_location_offs(pt3, 0, +1, 0, +1) == BBLocation::OutsideTop);
      assert(rec.find_location_offs(pt3, +1, 0, 0, +1) == BBLocation::Inside);
      assert(rec.find_location_offs(pt4, 0, +1, 0, +1) == BBLocation::OutsideLeft);
      assert(rec.find_location_offs(pt4, 0, +1, +1, +1) == BBLocation::Inside);
      
      assert(rec.is_empty());
      assert(!rec.is_inverted());
      assert(rec.is_collapsed());
      assert(!rec.is_valid());
      
      assert(rec.center() == RC(0, 0));
      assert(rec.r_mid() == 0);
      assert(rec.c_mid() == 0);
      assert(rec.top() == 0);
      assert(rec.bottom() == -1);
      assert(rec.left() == 0);
      assert(rec.right() == -1);
      assert(rec.pos() == RC(0, 0));
      assert(rec.size() == RC(0, 0));
      
      // No need to replicate these two tests for rectangles of other dimensions:
      auto rec_tmp = rec;
      rec_tmp.set_pos({ 2, -1 });
      rec_tmp.set_size({ 4, 4 });
      assert(rec_tmp.pos() == RC(2, -1));
      assert(rec_tmp.size() == RC(4, 4));
    }
    
    {
      Rectangle rec { -1, +1, 2, 1}; // [-1..0, 1]
      
      assert(rec.r == -1);
      assert(rec.c == +1);
      assert(rec.r_len == 2);
      assert(rec.c_len == 1);
      
      rec.clear();
      assert(rec.r == 0);
      assert(rec.c == 0);
      assert(rec.r_len == 0);
      assert(rec.c_len == 0);
      rec = { -1, +1, 2, 1};
      
      assert(!rec.is_inside(0, 0));
      assert(!rec.is_inside(1, 0));
      assert(rec.is_inside(0, 1));
      assert(!rec.is_inside(-1, 0));
      assert(!rec.is_inside(0, -1));
      assert(!rec.is_inside(pt0));
      assert(!rec.is_inside(pt1));
      assert(rec.is_inside(pt2));
      assert(!rec.is_inside(pt3));
      assert(!rec.is_inside(pt4));
      
      assert(!rec.is_inside_offs(0, 0, -1));
      assert(rec.is_inside_offs(0, 0, +1));
      assert(!rec.is_inside_offs(1, 0, -1));
      assert(rec.is_inside_offs(1, 0, +1));
      assert(!rec.is_inside_offs(0, 1, -1));
      assert(rec.is_inside_offs(0, 1, +1));
      assert(!rec.is_inside_offs(-1, 0, -1));
      assert(rec.is_inside_offs(-1, 0, +1));
      assert(!rec.is_inside_offs(0, -1, -1));
      assert(!rec.is_inside_offs(0, -1, +1));
      assert(rec.is_inside_offs(0, -1, +2));
      assert(!rec.is_inside_offs(pt0, -1));
      assert(rec.is_inside_offs(pt0, +1));
      assert(!rec.is_inside_offs(pt1, -1));
      assert(rec.is_inside_offs(pt1, +1));
      assert(!rec.is_inside_offs(pt2, -1));
      assert(rec.is_inside_offs(pt2, +1));
      assert(!rec.is_inside_offs(pt3, -1));
      assert(rec.is_inside_offs(pt3, +1));
      assert(!rec.is_inside_offs(pt4, -1));
      assert(!rec.is_inside_offs(pt4, +1));
      assert(rec.is_inside_offs(pt4, +2));
      
      assert(rec.is_inside_offs(pt0, +1, +2));
      assert(rec.is_inside_offs(pt1, +1, +2));
      assert(rec.is_inside_offs(pt2, +1, +2));
      assert(rec.is_inside_offs(pt3, +1, +2));
      assert(rec.is_inside_offs(pt4, +1, +2));
      
      assert(!rec.is_inside_offs(pt0, +1, +2, 0, +1));
      assert(rec.is_inside_offs(pt0, +1, +2, +1, 0));
      assert(!rec.is_inside_offs(pt0, +2, 0, 0, +1));
      assert(rec.is_inside_offs(pt0, 0, +1, +1, +1));
      assert(!rec.is_inside_offs(pt0, 0, +1, -1, +1));
      assert(!rec.is_inside_offs(pt0, -1, +1, -1, +1));
      assert(!rec.is_inside_offs(pt0, +1, -1, +1, -1));
      assert(rec.is_inside_offs(pt0, +1, +1, +1, +1));
      assert(rec.is_inside_offs(pt0, +2, +2, +2, +2));
      
      assert(!rec.is_inside_offs(pt1, +1, +2, 0, +1));
      assert(rec.is_inside_offs(pt1, +1, +2, +1, 0));
      assert(!rec.is_inside_offs(pt1, +2, 0, 0, +1));
      assert(rec.is_inside_offs(pt1, 0, +1, +1, +1));
      assert(!rec.is_inside_offs(pt1, 0, +1, -1, +1));
      assert(!rec.is_inside_offs(pt1, -1, +1, -1, +1));
      assert(!rec.is_inside_offs(pt1, +1, -1, +1, -1));
      assert(rec.is_inside_offs(pt1, +1, +1, +1, +1));
      assert(rec.is_inside_offs(pt1, +2, +2, +2, +2));
      
      assert(rec.is_inside_offs(pt2, +1, +2, 0, +1));
      assert(rec.is_inside_offs(pt2, +1, +2, +1, 0));
      assert(rec.is_inside_offs(pt2, +2, 0, 0, +1));
      assert(rec.is_inside_offs(pt2, 0, +1, +1, +1));
      assert(!rec.is_inside_offs(pt2, 0, +1, -1, +1));
      assert(!rec.is_inside_offs(pt2, -1, +1, -1, +1));
      assert(!rec.is_inside_offs(pt2, +1, -1, +1, -1));
      assert(rec.is_inside_offs(pt2, +1, +1, +1, +1));
      assert(rec.is_inside_offs(pt2, +2, +2, +2, +2));
      
      assert(!rec.is_inside_offs(pt3, +1, +2, 0, +1));
      assert(rec.is_inside_offs(pt3, +1, +2, +1, 0));
      assert(!rec.is_inside_offs(pt3, +2, 0, 0, +1));
      assert(rec.is_inside_offs(pt3, 0, +1, +1, +1));
      assert(!rec.is_inside_offs(pt3, 0, +1, -1, +1));
      assert(!rec.is_inside_offs(pt3, -1, +1, -1, +1));
      assert(rec.is_inside_offs(pt3, +1, -1, +1, -1));
      assert(rec.is_inside_offs(pt3, +1, +1, +1, +1));
      assert(rec.is_inside_offs(pt3, +2, +2, +2, +2));
      
      assert(!rec.is_inside_offs(pt4, +1, +2, 0, +1));
      assert(!rec.is_inside_offs(pt4, +1, +2, +1, 0));
      assert(!rec.is_inside_offs(pt4, +2, 0, 0, +1));
      assert(!rec.is_inside_offs(pt4, 0, +1, +1, +1));
      assert(!rec.is_inside_offs(pt4, 0, +1, -1, +1));
      assert(!rec.is_inside_offs(pt4, -1, +1, -1, +1));
      assert(!rec.is_inside_offs(pt4, +1, -1, +1, -1));
      assert(!rec.is_inside_offs(pt4, +1, +1, +1, +1));
      assert(rec.is_inside_offs(pt4, +2, +2, +2, +2));
      
      assert(rec.is_on_border(pt0));
      assert(!rec.is_on_border(pt1));
      assert(rec.is_on_border(pt2));
      assert(rec.is_on_border(pt3));
      assert(rec.is_on_border(pt4));
      
      assert(rec.find_location(pt0) == BBLocation::OutsideLeft);
      assert(rec.find_location(pt1) == BBLocation::OutsideBottomLeft);
      assert(rec.find_location(pt2) == BBLocation::Inside); // rec:[-1..0, 1], pt2:[0, 1]
      assert(rec.find_location(pt3) == BBLocation::OutsideLeft);
      assert(rec.find_location(pt4) == BBLocation::OutsideLeft);
      
      assert(rec.find_location_offs(pt0, 0, +1, 0, +1) == BBLocation::OutsideLeft);
      assert(rec.find_location_offs(pt0, -1, +1, 0, +1) == BBLocation::OutsideLeft);
      assert(rec.find_location_offs(pt1, 0, +1, 0, +1) == BBLocation::OutsideLeft);
      assert(rec.find_location_offs(pt1, 0, +2, 0, +1) == BBLocation::OutsideLeft);
      assert(rec.find_location_offs(pt2, 0, +1, 0, +1) == BBLocation::Inside);
      assert(rec.find_location_offs(pt2, 0, +1, 0, +2) == BBLocation::Inside);
      assert(rec.find_location_offs(pt3, 0, +1, 0, +1) == BBLocation::OutsideLeft);
      assert(rec.find_location_offs(pt3, +1, 0, 0, +1) == BBLocation::OutsideLeft);
      assert(rec.find_location_offs(pt4, 0, +1, 0, +1) == BBLocation::OutsideLeft);
      assert(rec.find_location_offs(pt4, 0, +1, +1, +1) == BBLocation::OutsideLeft);
      assert(rec.find_location_offs(pt4, -1, 0, -1, 0) == BBLocation::None); // rec:[-1..0, 1] -> [0, .], pt4:[0, -1]
      
      assert(!rec.is_empty());
      assert(!rec.is_inverted());
      assert(!rec.is_collapsed());
      assert(rec.is_valid());
      
      assert(rec.center() == RC(0, 1)); // rec:[-1..0, 1], { -1, +1, 2, 1}/2 = -1+1, 1+0.5
      assert(rec.r_mid() == 0);
      assert(rec.c_mid() == 1);
      assert(rec.top() == -1);
      assert(rec.bottom() == 0);
      assert(rec.left() == 1);
      assert(rec.right() == 1);
      assert(rec.pos() == RC(-1, +1));
      assert(rec.size() == RC(2, 1));
    }
  }
  
}
