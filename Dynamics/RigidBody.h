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
#include <Core/bool_vector.h>


namespace dynamics
{
  
  class RigidBody
  {
    AABB<int> curr_sprite_aabb;
    AABB<float> curr_aabb;
    bool_vector curr_coll_mask;
    int collision_material = 1;
    
    Vec2 orig_pos;
    Vec2 cm_to_orig_pos;
    
    Vec2 curr_centroid;
    Vec2 curr_cm;
    Vec2 curr_vel;
    Vec2 curr_acc;
    //float mass = 0.f;
    
    Sprite* sprite = nullptr; // Position to be controlled by this rigid body object.
    
  public:
    RigidBody(Sprite* s, const Vec2& vel = {}, const Vec2& acc = {}, int coll_mat = 1)
      : collision_material(coll_mat)
    {
      sprite = s;
      orig_pos = s->pos;
      curr_sprite_aabb = s->calc_curr_AABB(0);
      curr_aabb = curr_sprite_aabb.convert<float>();
      curr_coll_mask = sprite->calc_curr_coll_mask(0, collision_material);
      curr_centroid = s->calc_curr_centroid(0);
      int num_points = 0;
      int rmin = curr_sprite_aabb.r_min();
      int rmax = curr_sprite_aabb.r_max();
      int cmin = curr_sprite_aabb.c_min();
      int cmax = curr_sprite_aabb.c_max();
      for (int r = rmin; r <= rmax; ++r)
      {
        for (int c = cmin; c <= cmax; ++c)
        {
          int idx = (r - rmin) * curr_sprite_aabb.width() + (c - cmin);
          if (curr_coll_mask[idx])
          {
            curr_cm += { static_cast<float>(r), static_cast<float>(c) };
            num_points++;
          }
        }
      }
      curr_cm /= num_points;
      std::cout << curr_cm.str() << std::endl;
      cm_to_orig_pos = orig_pos - curr_cm;
      curr_vel = vel;
      curr_acc = acc;
    }
    
    void update(float dt, int sim_frame)
    {
      if (sprite != nullptr)
      {
        curr_vel += curr_acc * dt;
        curr_cm += curr_vel * dt;
        curr_centroid += curr_vel * dt;
        sprite->pos = (curr_cm + cm_to_orig_pos).to_RC_round();
        curr_sprite_aabb = sprite->calc_curr_AABB(sim_frame);
        curr_aabb = curr_sprite_aabb.convert<float>();
        curr_coll_mask = sprite->calc_curr_coll_mask(sim_frame, collision_material);
      }
    }
    
    Vec2 get_curr_cm() const { return curr_cm; }
    
    Vec2 get_curr_centroid() const { return curr_centroid; }
    
    AABB<float> get_curr_AABB() const { return curr_aabb; }
  };
  
  
}
