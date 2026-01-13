//
//  RigidBody.h
//  Termin8or
//
//  Created by Rasmus Anthin on 2024-10-22.
//

#pragma once
#include "../../geom/Rectangle.h"
#include "../../geom/RC.h"
#include "../../sprite/SpriteHandler.h"
#include <Core/bool_vector.h>
#include <Core/MathUtils.h>
#include <Core/Mtx2.h>


namespace t8x
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
    Vec2 orig_dir { 0.f, 1.f };
    
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
    float dynamic_friction = 0.f;
    std::optional<float> critical_speed_r = std::nullopt;
    std::optional<float> critical_speed_c = std::nullopt;
    
    AABB<int> curr_sprite_aabb;
    AABB<float> curr_aabb;
    bool_vector curr_inertia_mask, curr_coll_mask;
    std::vector<uint8_t> inertia_materials { 1 };
    std::vector<uint8_t> collision_materials { 1 };
    std::vector<Vec2> surface_normals;
    
    bool enable_sleeping = false;
    bool sleeping = false;
    float awake_impulse_threshold_sq = 1.f;
    float sleep_velocity_threshold_sq = 0.1f;
    float sleep_time_threshold = 5.f;
    float sleep_timestamp = 0.f;
    
    void calc_cm_and_I(int sim_frame)
    {
      curr_cm_local = { 0.f, 0.f };
      curr_sprite_aabb = sprite->calc_curr_AABB(sim_frame);
      curr_coll_mask = sprite->calc_curr_mask(sim_frame, collision_materials);
      curr_inertia_mask = sprite->calc_curr_mask(sim_frame, inertia_materials);
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
          if (curr_inertia_mask[idx])
          {
            if (sprite->calc_cm())
              curr_cm_local += { static_cast<float>(r_loc), static_cast<float>(c_loc) };
            Ixx += static_cast<float>(math::sq(r_loc));
            Iyy += static_cast<float>(math::sq(c_loc));
            num_points++;
          }
        }
      }
      if (sprite->calc_cm())
        curr_cm_local /= static_cast<float>(num_points);
      
      auto density = mass / num_points;
      Ixx *= density;
      Iyy *= density;
      Iz = Ixx + Iyy;
      inv_Iz = Iz == 0.f ? 0.f : 1.f / Iz;
    }
    
    void calc_surface_normals()
    {
      int rmin = curr_sprite_aabb.r_min();
      int rmax = curr_sprite_aabb.r_max();
      int cmin = curr_sprite_aabb.c_min();
      int cmax = curr_sprite_aabb.c_max();
      for (int r = rmin; r <= rmax; ++r)
      {
        auto r_loc = r - rmin;
        for (int c = cmin; c <= cmax; ++c)
        {
          //auto pos = to_Vec2({ r, c });
          //auto cm_dir = math::normalize(pos - curr_cm_local);
        
          auto c_loc = c - cmin;
          
          auto idx = r_loc * curr_sprite_aabb.width() + c_loc;
          
          if (!curr_coll_mask[idx])
            continue;
          
          Vec2 normal;
          for (int i = -1; i <= 1; ++i)
          {
            for (int j = -1; j <= 1; ++j)
            {
              if (i == 0 && j == 0)
                continue;
              auto offs_pos = RC { r + i, c + j };
              if (curr_sprite_aabb.contains(offs_pos))
              {
                auto idx_offs = (r_loc + i) * curr_sprite_aabb.width() + (c_loc + j);
                if (curr_coll_mask[idx_offs])
                {
                  auto offs_dir = t8::to_Vec2({ i, j });
                  //if (math::dot(math::normalize(offs_dir), cm_dir) > 0.f)
                    normal -= offs_dir;
                }
              }
            }
          }
          stlutils::at_growing(surface_normals, idx) = math::normalize(normal);
        }
      }
    }
    
  public:
    RigidBody(Sprite* s, float rb_mass = 1.f,
      std::optional<Vec2> pos = std::nullopt, const Vec2& vel = {}, const Vec2& force = {},
      float ang_vel = 0.f, float torque = 0.f,
      float e = 0.8f, float dyn_friction = 0.f,
      std::optional<float> crit_speed_r = std::nullopt,
      std::optional<float> crit_speed_c = std::nullopt,
      const std::vector<uint8_t>& inertia_mats = { 1 },
      const std::vector<uint8_t>& coll_mats = { 1 })
      : sprite(s)
      , mass(rb_mass)
      , inv_mass(rb_mass == 0.f ? 0.f : 1.f / rb_mass)
      , curr_vel(vel)
      , curr_force(force)
      , curr_ang_vel(ang_vel)
      , curr_torque(torque)
      , coeff_of_restitution(e)
      , dynamic_friction(math::clamp<float>(dyn_friction, 0.f, 1.f))
      , critical_speed_r(crit_speed_r)
      , critical_speed_c(crit_speed_c)
      , inertia_materials(inertia_mats)
      , collision_materials(coll_mats)
    {
      //std::cout << "name: " << s->get_name() << std::endl;
      //std::cout << "pos: " << s->pos.str() << std::endl;
      orig_pos = pos.value_or(to_Vec2(s->pos));
      calc_cm_and_I(0);
      calc_surface_normals();
      orig_cm_local = curr_cm_local;
      curr_cm = orig_pos + curr_cm_local;
      curr_aabb = curr_sprite_aabb.convert<float>();
      curr_centroid = s->calc_curr_centroid(0);
      cm_to_orig_pos = orig_pos - curr_cm;
      if (auto* vector_sprite = dynamic_cast<VectorSprite*>(sprite); vector_sprite != nullptr)
        curr_ang = math::deg2rad(vector_sprite->get_rotation());
    }
    
    void update(float time, float dt, int sim_frame)
    {
      if (sprite != nullptr)
      {
        if (mass > 0.f && !(enable_sleeping && sleeping))
        {
          curr_acc = curr_force * inv_mass;
          curr_vel += curr_acc * dt;
          if (critical_speed_r.has_value() && std::abs(curr_vel.r) > std::abs(critical_speed_r.value()))
            curr_vel.r = math::sgn(curr_vel.r)*std::abs(critical_speed_r.value());
          if (critical_speed_c.has_value() && std::abs(curr_vel.c) > std::abs(critical_speed_c.value()))
            curr_vel.c = math::sgn(curr_vel.c)*std::abs(critical_speed_c.value());
          curr_cm += curr_vel * dt;
          curr_centroid += curr_vel * dt;
          curr_ang_acc = curr_torque * inv_Iz;
          curr_ang_vel += curr_ang_acc * dt;
          curr_ang += curr_ang_vel * dt;
          
          // curr_cm + (orig_pos - orig_cm) + (orig_cm_local - curr_cm_local)
          auto sprite_pos = curr_cm + cm_to_orig_pos + (curr_cm_local - orig_cm_local);
          sprite->pos = t8::to_RC_round(sprite_pos);
          if (auto* vector_sprite = dynamic_cast<VectorSprite*>(sprite); vector_sprite != nullptr)
            vector_sprite->set_rotation(math::rad2deg(curr_ang));
          
          if (enable_sleeping)
          {
            if (math::length_squared(curr_vel) < sleep_velocity_threshold_sq)
            {
              if (time - sleep_timestamp > sleep_time_threshold)
                sleeping = true;
            }
            else
              sleep_timestamp = time;
          }
        }
        
        calc_cm_and_I(sim_frame);
        calc_surface_normals();
        curr_aabb = curr_sprite_aabb.convert<float>();
      }
    }
    
    Vec2 get_curr_cm() const { return curr_cm; }
    
    float get_curr_cm_r() const { return curr_cm.r; }
    
    float get_curr_cm_c() const { return curr_cm.c; }
    
    void set_curr_cm(const Vec2& cm) { curr_cm = cm; }
    
    void set_curr_cm_r(float row_loc) { curr_cm.r = row_loc; }
    
    void set_curr_cm_c(float col_loc) { curr_cm.c = col_loc; }
    
    void reset_curr_cm() { curr_cm = orig_pos + curr_cm_local; }
    
    Vec2 get_curr_centroid() const { return curr_centroid; }
    
    AABB<float> get_curr_AABB() const { return curr_aabb; }
    
    const bool_vector& get_curr_inertia_mask() const { return curr_inertia_mask; }
    
    const bool_vector& get_curr_coll_mask() const { return curr_coll_mask; }
    
    const Vec2 fetch_surface_normal(const RC& local_pos) const
    {
      auto idx = local_pos.r * curr_sprite_aabb.width() + local_pos.c;
      if (idx < stlutils::sizeI(surface_normals))
        return surface_normals[idx];
      return {};
    }
    
    // Elasticity coefficient.
    float get_e() const { return coeff_of_restitution; }
    
    // pt in world ref frame.
    Vec2 calc_velocity_at(const Vec2& pt) const
    {
      auto r = pt - curr_cm;
      auto v_ang = Vec2 { r.c, -r.r } * curr_ang_vel;
      return curr_vel + v_ang;
    }
    
    Vec2 get_curr_lin_force() const { return curr_force; }
    
    void set_curr_lin_force(const Vec2& force) { curr_force = force; }
    
    Vec2 get_curr_lin_vel() const { return curr_vel; }
    
    float get_curr_lin_speed_r() const { return curr_vel.r; }
    
    float get_curr_lin_speed_c() const { return curr_vel.c; }
    
    void set_curr_lin_vel(const Vec2& vel) { curr_vel = vel; }
    
    void set_curr_lin_speed_r(float row_speed) { curr_vel.r = row_speed; }
    
    void set_curr_lin_speed_c(float col_speed) { curr_vel.c = col_speed; }
    
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
      
      if (enable_sleeping)
      {
        if (math::length_squared(impulse) > awake_impulse_threshold_sq)
          sleeping = false;
      }
    }
    
    // w [rad/s]
    float get_curr_ang_vel() const
    {
      return curr_ang_vel;
    }
    
    // w [rad/s]
    void set_curr_ang_vel(float w_rad_s)
    {
      curr_ang_vel = w_rad_s;
    }
    
    // phi [rad]
    float get_curr_ang() const
    {
      return curr_ang;
    }
    
    // phi [rad]
    void set_curr_ang(float phi_rad)
    {
      curr_ang = phi_rad;
    }
    
    Mtx2 get_curr_rot_mtx() const
    {
      return Mtx2
        {
          std::cos(curr_ang), -std::sin(curr_ang),
          std::sin(curr_ang), std::cos(curr_ang)
        };
    }
    
    Vec2 get_orig_dir() const
    {
      return orig_dir;
    }
    
    void set_orig_dir(const Vec2& dir)
    {
      orig_dir = math::normalize(dir);
    }
    
    Vec2 get_curr_dir() const
    {
      return get_curr_rot_mtx() * orig_dir;
    }
    
    float get_dynamic_friction() const { return dynamic_friction; }
    
    void set_sleeping(bool enable,
                      float a_sleep_velocity_threshold,
                      float a_sleep_time_threshold,
                      float a_awake_impulse_threshold)
    {
      enable_sleeping = enable;
      sleep_velocity_threshold_sq = math::sq(a_sleep_velocity_threshold);
      sleep_time_threshold = a_sleep_time_threshold;
      awake_impulse_threshold_sq = math::sq(a_awake_impulse_threshold);
    }
        
    bool is_sleeping() const { return enable_sleeping && sleeping; }
    
    Sprite* get_sprite() const { return sprite; }
  };
  
  
}
