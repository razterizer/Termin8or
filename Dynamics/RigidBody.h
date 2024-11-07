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
    float curr_ang = 0.f;
    
    float mass = 1.f;
    float inv_mass = 1.f;
    //Mtx2 curr_I;
    float Iz = 1.f;
    float inv_Iz = 1.f;
    
    Vec2 curr_vel;
    Vec2 curr_acc;
    Vec2 curr_force;
    float curr_ang_vel = 0.f;
    float curr_ang_acc = 0.f;
    float curr_torque = 0.f;
    
    float coeff_of_restitution = 0.8f;
    
    AABB<int> curr_sprite_aabb;
    AABB<float> curr_aabb;
    bool_vector curr_coll_mask;
    int collision_material = 1;
    
    void calc_cm_and_I(int sim_frame)
    {
      curr_cm_local = { 0.f, 0.f };
      curr_sprite_aabb = sprite->calc_curr_AABB(sim_frame);
      curr_coll_mask = sprite->calc_curr_coll_mask(sim_frame, collision_material);
      int num_points = 0;
      int rmin = curr_sprite_aabb.r_min();
      int rmax = curr_sprite_aabb.r_max();
      int cmin = curr_sprite_aabb.c_min();
      int cmax = curr_sprite_aabb.c_max();
      float Ixx = 0.f, Iyy = 0.f;
      for (int r = rmin; r <= rmax; ++r)
      {
        auto r_loc = r - rmin;
        for (int c = cmin; c <= cmax; ++c)
        {
          auto c_loc = c - cmin;
          auto idx = r_loc * curr_sprite_aabb.width() + c_loc;
          if (curr_coll_mask[idx])
          {
            if (sprite->calc_cm())
              curr_cm_local += { static_cast<float>(r_loc), static_cast<float>(c_loc) };
            Ixx += math::sq<float>(r_loc);
            Iyy += math::sq<float>(c_loc);
            num_points++;
          }
        }
      }
      if (sprite->calc_cm())
        curr_cm_local /= num_points;
      
      auto density = mass / num_points;
      Ixx *= density;
      Iyy *= density;
      Iz = Ixx + Iyy;
      inv_Iz = 1.f / Iz;
    }
    
  public:
    RigidBody(Sprite* s, float rb_mass = 1.f,
      const Vec2& vel = {}, const Vec2& force = {},
      float ang_vel = 0.f, float torque = 0.f,
      float e = 0.8f, int coll_mat = 1)
      : sprite(s)
      , mass(rb_mass)
      , inv_mass(1.f / rb_mass)
      , curr_vel(vel)
      , curr_force(force)
      , curr_ang_vel(ang_vel)
      , curr_torque(torque)
      , coeff_of_restitution(e)
      , collision_material(coll_mat)
    {
      //std::cout << "name: " << s->get_name() << std::endl;
      //std::cout << "pos: " << s->pos.str() << std::endl;
      orig_pos = to_Vec2(s->pos);
      calc_cm_and_I(0);
      orig_cm_local = curr_cm_local;
      curr_cm = orig_pos + curr_cm_local;
      curr_aabb = curr_sprite_aabb.convert<float>();
      curr_centroid = s->calc_curr_centroid(0);
      cm_to_orig_pos = orig_pos - curr_cm;
    }
    
    void update(float dt, int sim_frame)
    {
      if (sprite != nullptr)
      {
        curr_acc = curr_force / mass;
        curr_vel += curr_acc * dt;
        curr_cm += curr_vel * dt;
        curr_centroid += curr_vel * dt;
        curr_ang_acc = curr_torque / Iz;
        curr_ang_vel += curr_ang_acc * dt;
        curr_ang += curr_ang_vel * dt;
        
        // curr_cm + (orig_pos - orig_cm) + (orig_cm_local - curr_cm_local)
        auto sprite_pos = curr_cm + cm_to_orig_pos + (curr_cm_local - orig_cm_local);
        sprite->pos = to_RC_round(sprite_pos);
        if (auto* vector_sprite = dynamic_cast<VectorSprite*>(sprite); vector_sprite != nullptr)
          vector_sprite->set_rotation(math::rad2deg(curr_ang));
        
        calc_cm_and_I(sim_frame);
        curr_aabb = curr_sprite_aabb.convert<float>();
      }
    }
    
    Vec2 get_curr_cm() const { return curr_cm; }
    
    Vec2 get_curr_centroid() const { return curr_centroid; }
    
    AABB<float> get_curr_AABB() const { return curr_aabb; }
    
    const bool_vector& get_curr_coll_mask() const { return curr_coll_mask; }
    
    // Elasticity coefficient.
    float get_e() const { return coeff_of_restitution; }
    
    // pt in world ref frame.
    Vec2 calc_velocity_at(const Vec2& pt) const
    {
      auto r = pt - curr_cm;
      auto v_ang = Vec2 { r.c, -r.r } * curr_ang_vel;
      return curr_vel + v_ang;
    }
    
    float get_inv_mass() const { return inv_mass; }
    
    float get_inv_Iz() const { return inv_Iz; }
    
    // pt in world ref frame.
    void apply_impulse(const Vec2& impulse, const Vec2& pt)
    {
      auto delta_v = impulse * inv_mass;
      curr_vel += delta_v;
    
      auto r = pt - curr_cm;
      auto tau = r.c * impulse.r - r.r * impulse.c;
      auto delta_w = tau * inv_Iz;
      curr_ang_vel += delta_w;
    }
  };
  
  
}
