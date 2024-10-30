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
    Sprite* sprite = nullptr; // Position to be controlled by this rigid body object.
    
    Vec2 orig_pos; // world location of sprite handle pos
    Vec2 cm_to_orig_pos;
    Vec2 curr_centroid; // world pos
    Vec2 orig_cm_local; // local pos
    Vec2 curr_cm_local; // local pos
    Vec2 curr_cm;
    
    Vec2 curr_pos; // world location of sprite handle pos
    Vec2 curr_vel;
    Vec2 curr_acc;
    //float mass = 0.f;
    
    AABB<int> curr_sprite_aabb;
    AABB<float> curr_aabb;
    bool_vector curr_coll_mask;
    int collision_material = 1;
    
    void calc_cm(int sim_frame)
    {
      curr_sprite_aabb = sprite->calc_curr_AABB(sim_frame);
      curr_coll_mask = sprite->calc_curr_coll_mask(sim_frame, collision_material);
      int num_points = 0;
      int rmin = curr_sprite_aabb.r_min();
      int rmax = curr_sprite_aabb.r_max();
      int cmin = curr_sprite_aabb.c_min();
      int cmax = curr_sprite_aabb.c_max();
      for (int r = rmin; r <= rmax; ++r)
      {
        auto r_loc = r - rmin;
        for (int c = cmin; c <= cmax; ++c)
        {
          auto c_loc = c - cmin;
          auto idx = r_loc * curr_sprite_aabb.width() + c_loc;
          if (curr_coll_mask[idx])
          {
            curr_cm_local += { static_cast<float>(r_loc), static_cast<float>(c_loc) };
            num_points++;
          }
        }
      }
      curr_cm_local /= num_points;
      curr_cm_local += sprite->get_pos_offs(sim_frame);
    }
    
  public:
    RigidBody(Sprite* s, const Vec2& vel = {}, const Vec2& acc = {}, int coll_mat = 1)
      : sprite(s)
      , curr_vel(vel)
      , curr_acc(acc)
      , collision_material(coll_mat)
    {
      std::cout << "name: " << s->get_name() << std::endl;
      std::cout << "pos: " << s->pos.str() << std::endl;
      orig_pos = curr_pos = to_Vec2(s->pos);
      calc_cm(0);
      orig_cm_local = curr_cm_local;
      curr_cm = curr_pos + curr_cm_local;
      curr_aabb = curr_sprite_aabb.convert<float>();
      curr_centroid = s->calc_curr_centroid(0);
      cm_to_orig_pos = orig_pos - curr_cm;
    }
    
    void update(float dt, int sim_frame)
    {
      if (sprite != nullptr)
      {
        calc_cm(sim_frame);
        curr_aabb = curr_sprite_aabb.convert<float>();
        curr_vel += curr_acc * dt;
        //curr_pos += curr_vel * dt;
        curr_cm += curr_vel * dt;
        curr_centroid += curr_vel * dt;
        // curr_cm + (orig_pos - orig_cm) + (orig_cm_local - curr_cm_local)
        auto sprite_pos = curr_cm + cm_to_orig_pos + (curr_cm_local - orig_cm_local);
        sprite->pos = to_RC_round(sprite_pos);
      }
    }
    
    Vec2 get_curr_cm() const { return curr_cm; }
    
    Vec2 get_curr_centroid() const { return curr_centroid; }
    
    AABB<float> get_curr_AABB() const { return curr_aabb; }
  };
  
  
}
