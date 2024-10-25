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
  
  struct RigidBody
  {
    AABB<float> curr_aabb;
    Vec2 orig_pos;
    Vec2 curr_centroid;
    Vec2 centroid_to_orig_pos;
    
    Sprite* sprite = nullptr; // Position to be controlled by this rigid body object.
    
    void init_from_sprite(Sprite* s)
    {
      sprite = s;
      orig_pos = s->pos;
      curr_aabb = s->calc_curr_AABB(0).convert<float>();
      curr_centroid = s->calc_curr_centroid(0);
      centroid_to_orig_pos = orig_pos - curr_centroid;
    }
    
    void update(int sim_frame)
    {
      if (sprite != nullptr)
      {
        //sprite->pos = curr_pos.to_RC_round();
        sprite->pos = (curr_centroid + centroid_to_orig_pos).to_RC_round();
        curr_aabb = sprite->calc_curr_AABB(sim_frame).convert<float>();
      }
    }
  };
  
  
}
