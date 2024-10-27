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
    RigidBody* rigid_body = nullptr;
    std::vector<std::unique_ptr<BVH_Node>> children;
    int level = 0;
    
    // Recursive build function
    void build(const AABB<float>& aabb_parent, std::vector<RigidBody*> rigid_bodies, int lvl)
    {
      aabb = aabb_parent;
      level = lvl;
            
      // Base case: If there's only one body, this node becomes a leaf node
      if (rigid_bodies.size() <= 1)
      {
        rigid_body = rigid_bodies.empty() ? nullptr : rigid_bodies[0];
        return;
      }
      
      // Calculate centroid AABB to determine split axis
      AABB<float> centroid_aabb;
      for (const auto* rb : rigid_bodies)
        centroid_aabb.add_point(rb->get_curr_centroid());
      int split_axis = centroid_aabb.width() > centroid_aabb.height() ? 1 : 0;
      
      // Sort bodies by centroid on the chosen axis and split in half
      auto it_mid = rigid_bodies.begin() + rigid_bodies.size() / 2;
      std::nth_element(rigid_bodies.begin(), it_mid, rigid_bodies.end(),
                       [split_axis](const RigidBody* a, const RigidBody* b)
      {
        return split_axis == 0 ?
          a->get_curr_centroid().r < b->get_curr_centroid().r :
          a->get_curr_centroid().c < b->get_curr_centroid().c;
      });
      
      // Divide into two subsets
      std::vector<RigidBody*> lower_rigid_bodies(rigid_bodies.begin(), it_mid);
      std::vector<RigidBody*> upper_rigid_bodies(it_mid, rigid_bodies.end());
      
      AABB<float> ch0_aabb;
      for (const auto* rb : lower_rigid_bodies)
        ch0_aabb.grow(rb->get_curr_AABB());
      AABB<float> ch1_aabb;
      for (const auto* rb : upper_rigid_bodies)
        ch1_aabb.grow(rb->get_curr_AABB());
      
      auto& ch0 = children.emplace_back(std::make_unique<BVH_Node>());
      ch0->build(ch0_aabb, lower_rigid_bodies, lvl + 1);
      
      auto& ch1 = children.emplace_back(std::make_unique<BVH_Node>());
      ch1->build(ch1_aabb, upper_rigid_bodies, lvl + 1);
    }
    
    template<int NR, int NC>
    void draw(ScreenHandler<NR, NC>& sh) const
    {
      auto rec = aabb.to_rectangle();
      auto color = color::colors_hue_light_dark[static_cast<size_t>(level) % color::colors_hue_light_dark.size()];
      drawing::draw_box_outline(sh, rec, drawing::OutlineType::Line, { color, Color::Transparent2 });
      for (const auto& ch : children)
        ch->draw(sh);
    }
  };
  
  class CollisionHandler
  {
    std::unique_ptr<BVH_Node> m_aabb_bvh;
        
  public:
    CollisionHandler()
    {
      m_aabb_bvh = std::make_unique<BVH_Node>();
    }
    
    void rebuild_AABB_BVH(int NR, int NC,
                          const DynamicsSystem* dyn_sys)
    {
      m_aabb_bvh->children.clear();
      auto NRf = static_cast<float>(NR);
      auto NCf = static_cast<float>(NC);
      AABB<float> aabb { 0.f, 0.f, NRf, NCf };
      auto rigid_bodies = dyn_sys->get_rigid_bodies_raw();
      m_aabb_bvh->build(aabb, rigid_bodies, 0);
    }
    
    template<int NR, int NC>
    void draw_AABB_BVH(ScreenHandler<NR, NC>& sh) const
    {
      m_aabb_bvh->draw(sh);
    }
    
    void update_detection()
    {
      
    }
    
    
  };
  
}
