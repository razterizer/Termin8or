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
  
  class RigidBody
  {
    AABB<float> curr_aabb;
    Vec2 orig_pos;
    Vec2 centroid_to_orig_pos;
    
    Vec2 curr_centroid;
    Vec2 curr_vel;
    Vec2 curr_acc;
    //float mass = 0.f;
    
    Sprite* sprite = nullptr; // Position to be controlled by this rigid body object.
    
  public:
    RigidBody(Sprite* s, const Vec2& vel = {}, const Vec2& acc = {})
    {
      sprite = s;
      orig_pos = s->pos;
      curr_aabb = s->calc_curr_AABB(0).convert<float>();
      curr_centroid = s->calc_curr_centroid(0);
      centroid_to_orig_pos = orig_pos - curr_centroid;
      curr_vel = vel;
      curr_acc = acc;
    }
    
    void update(float dt, int sim_frame)
    {
      if (sprite != nullptr)
      {
        curr_vel += curr_acc * dt;
        curr_centroid += curr_vel * dt;
        //sprite->pos = curr_pos.to_RC_round();
        sprite->pos = (curr_centroid + centroid_to_orig_pos).to_RC_round();
        curr_aabb = sprite->calc_curr_AABB(sim_frame).convert<float>();
      }
    }
    
    Vec2 get_curr_centroid() const { return curr_centroid; }
    
    AABB<float> get_curr_AABB() const { return curr_aabb; }
  };
  
  
}
