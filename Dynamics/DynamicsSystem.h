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
  
    RigidBody* add_rigid_body(Sprite* sprite, float rb_mass = 0.f,
      const Vec2& vel = {}, const Vec2& force = {},
      float ang_vel = 0.f, float torque = 0.f,
      int coll_mat = 1)
    {
      auto& rb = m_rigid_bodies.emplace_back(std::make_unique<RigidBody>(sprite, rb_mass,
        vel, force,
        ang_vel, torque,
        coll_mat));
      return rb.get();
    }
    
    const std::vector<RigidBody*> get_rigid_bodies_raw() const
    {
      std::vector<RigidBody*> rigid_bodies_raw;
      rigid_bodies_raw.reserve(m_rigid_bodies.size());
      for (const auto& rb : m_rigid_bodies)
        rigid_bodies_raw.emplace_back(rb.get());
      return rigid_bodies_raw;
    }
    
    void update(float dt, int sim_frame)
    {
      for (auto& rb : m_rigid_bodies)
        rb->update(dt, sim_frame);
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
