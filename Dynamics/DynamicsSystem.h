//
//  DynamicsSystem.h
//  Termin8or
//
//  Created by Rasmus Anthin on 2024-10-25.
//

#pragma once
#include "RigidBody.h"
#include "../SpriteHandler.h"
#include <vector>
#include <memory>


namespace dynamics
{

  class DynamicsSystem
  {
    std::vector<std::unique_ptr<RigidBody>> m_rigid_bodies;
  
  public:
  
    RigidBody* add_rigid_body(Sprite* sprite, float rb_mass = 1.f,
      std::optional<Vec2> pos = std::nullopt, const Vec2& vel = {}, const Vec2& force = {},
      float ang_vel = 0.f, float torque = 0.f,
      float e = 0.8f, float dyn_friction = 0.f,
      std::optional<float> crit_speed_r = std::nullopt,
      std::optional<float> crit_speed_c = std::nullopt,
      const std::vector<int>& inertia_mats = { 1 },
      const std::vector<int>& coll_mats = { 1 })
    {
      auto& rb = m_rigid_bodies.emplace_back(std::make_unique<RigidBody>(sprite, rb_mass,
        pos, vel, force,
        ang_vel, torque,
        e, dyn_friction,
        crit_speed_r, crit_speed_c,
        inertia_mats, coll_mats));
      return rb.get();
    }
    
    void remove_rigid_body(RigidBody* rb)
    {
      stlutils::erase_if(m_rigid_bodies, [rb](const auto& rb_uptr) { return rb_uptr.get() == rb; });
    }
    
    template<int NRB>
    std::array<RigidBody*, NRB> add_rigid_bodies(const std::array<Sprite*, NRB>& sprites,
      std::function<float(int)> rb_mass = [](int){ return 1.f; },
      std::function<std::optional<Vec2>(int)> pos = [](int){ return std::nullopt; },
      std::function<Vec2(int)> vel = [](int){ return Vec2 {}; },
      std::function<Vec2(int)> force = [](int){ return Vec2 {}; },
      std::function<float(int)> ang_vel = [](int){ return 0.f; },
      std::function<float(int)> torque = [](int){ return 0.f; },
      std::function<float(int)> e = [](int){ return 0.8f; },
      std::function<float(int)> dyn_friction = [](int) { return 0.f; },
      std::function<std::optional<float>(int)> crit_speed_r = [](int){ return std::nullopt; },
      std::function<std::optional<float>(int)> crit_speed_c = [](int){ return std::nullopt; },
      std::function<std::vector<int>(int)> inertia_mats = [](int){ return std::vector { 1 }; },
      std::function<std::vector<int>(int)> coll_mats = [](int){ return std::vector { 1 }; })
    {
      std::array<RigidBody*, NRB> rigid_body_arr;
      for (int s_idx = 0; s_idx < NRB; ++s_idx)
      {
        rigid_body_arr[s_idx] = add_rigid_body(sprites[s_idx], rb_mass(s_idx),
          pos(s_idx), vel(s_idx), force(s_idx),
          ang_vel(s_idx), torque(s_idx),
          e(s_idx), dyn_friction(s_idx),
          crit_speed_r(s_idx), crit_speed_c(s_idx),
          inertia_mats(s_idx), coll_mats(s_idx));
      }
      return rigid_body_arr;
    }
    
    const std::vector<RigidBody*> get_rigid_bodies_raw() const
    {
      std::vector<RigidBody*> rigid_bodies_raw;
      rigid_bodies_raw.reserve(m_rigid_bodies.size());
      for (const auto& rb : m_rigid_bodies)
        rigid_bodies_raw.emplace_back(rb.get());
      return rigid_bodies_raw;
    }
    
    void update(float time, float dt, int sim_frame)
    {
      for (auto& rb : m_rigid_bodies)
        rb->update(time, dt, sim_frame);
    }
    
    template<int NR, int NC>
    void draw_dbg(ScreenHandler<NR, NC>& sh)
    {
      for (auto& rb : m_rigid_bodies)
      {
        const auto& cm = rb->get_curr_cm();
        sh.write_buffer("+", math::roundI(cm.r), math::roundI(cm.c), Color::Cyan);
      }
    }
  };

}
