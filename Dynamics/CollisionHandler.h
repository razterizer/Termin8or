//
//  CollisionHandler.h
//  Termin8or
//
//  Created by Rasmus Anthin on 2024-10-21.
//

#pragma once
#include "../Rectangle.h"
#include "RigidBody.h"
#include "DynamicsSystem.h"


namespace dynamics
{

  struct BVH_Node
  {
    AABB<float> aabb;
    RC centroid;
    RigidBody* rigid_body = nullptr;
    std::vector<std::unique_ptr<BVH_Node>> children;
  };
  
  class CollisionHandler
  {
    std::unique_ptr<BVH_Node> m_aabb_bvh;
        
  public:
    CollisionHandler()
    {
      m_aabb_bvh = std::make_unique<BVH_Node>();
    }
    
    void rebuild_AABB_bvh(int NR, int NC,
                          const DynamicsSystem* dyn_sys)
    {
      m_aabb_bvh->children.clear();
      m_aabb_bvh->aabb = { 0.f, 0.f, static_cast<float>(NR), static_cast<float>(NC) };
    }
    
    void update_detection()
    {
      
    }
    
    
  };
  
}
