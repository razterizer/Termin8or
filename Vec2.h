//
//  Vec2.h
//  Termin8or
//
//  Created by Rasmus Anthin on 2024-10-25.
//

#pragma once
#include "RC.h"


struct Vec2
{
  float r = 0.f;
  float c = 0.f;
  
  Vec2(const Vec2& v)
  {
    r = v.r;
    c = v.c;
  }
  
  Vec2(const RC& p)
  {
    r = static_cast<float>(p.r);
    c = static_cast<float>(p.c);
  }
  
  operator RC() { return { math::roundI(r), math::roundI(c) }; }
};
