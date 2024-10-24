//
//  RigidBody.h
//  Termin8or
//
//  Created by Rasmus Anthin on 2024-10-22.
//

#pragma once
#include "../Rectangle.h"
#include "../RC.h"
#include "../SpriteHandler.h"


namespace dynamics
{

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
  
  struct RigidBody
  {
    AABB<float> aabb;
    Vec2 curr_pos;
    
    Sprite* sprite = nullptr; // Position to be controlled by this rigid body object.
    
    void init_from_sprite(Sprite* s)
    {
      sprite = s;
      curr_pos = s->pos;
      aabb = s->calc_curr_AABB(0).convert<float>();
    }
    
    void update(int sim_frame)
    {
      if (sprite != nullptr)
      {
        sprite->pos = curr_pos;
        aabb = sprite->calc_curr_AABB(sim_frame).convert<float>();
      }
    }
  };
  
  
}
