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
  
    RigidBody* add_rigid_body(Sprite* sprite)
    {
      auto& rb = m_rigid_bodies.emplace_back(std::make_unique<RigidBody>());
      rb->init_from_sprite(sprite);
      return rb.get();
    }
    
    void update()
    {
      
    }
  };

}
