//
//  RigidBody.h
//  Termin8or
//
//  Created by Rasmus Anthin on 2024-10-22.
//

#pragma once
#include "../Rectangle.h"
#include "../RC.h"


namespace dynamics
{

  struct Vec2
  {
    float r = 0.f;
    float c = 0.f;
  }
  
  struct RigidBody
  {
    ttl::Rectangle aabb;
    Vec2 curr_pos;
  };
  
  
}
