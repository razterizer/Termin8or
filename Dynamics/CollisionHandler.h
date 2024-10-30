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
#include <unordered_set>


namespace dynamics
{
  
  struct BVH_Node
  {
    AABB<float> aabb;
    RigidBody* rigid_body = nullptr;
    std::vector<std::unique_ptr<BVH_Node>> children;
    int level = 0;
    int order = -1;
    
    // Recursive build function
    void build(const AABB<float>& aabb_parent, std::vector<RigidBody*> rigid_bodies,
               std::vector<BVH_Node*>& leaves,
               int lvl, int ord = -1)
    {
      aabb = aabb_parent;
      level = lvl;
      order = ord;
            
      // Base case: If there's only one body, this node becomes a leaf node
      if (rigid_bodies.size() <= 1)
      {
        rigid_body = rigid_bodies.empty() ? nullptr : rigid_bodies[0];
        leaves.emplace_back(this);
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
      std::vector<RigidBody*> first_rigid_bodies(rigid_bodies.begin(), it_mid);
      std::vector<RigidBody*> last_rigid_bodies(it_mid, rigid_bodies.end());
      auto rb_first_mid_aabb = (*(it_mid-1))->get_curr_AABB();
      auto rb_last_mid_aabb = (*it_mid)->get_curr_AABB();
      
      // Define child AABBs for split axis
      if (split_axis == 0) // Horizontal split
      {
        AABB<float> first_aabb =
          { aabb.r_min(), aabb.c_min(), rb_first_mid_aabb.r_max() - aabb.r_min(), aabb.width() };
        AABB<float> last_aabb =
          { rb_last_mid_aabb.r_min(), aabb.c_min(), aabb.r_max() - rb_last_mid_aabb.r_min(), aabb.width() };
        
        // Create left and right children and build recursively
        auto& ch0 = children.emplace_back(std::make_unique<BVH_Node>());
        ch0->build(first_aabb, first_rigid_bodies, leaves, lvl + 1, 0);
        
        auto& ch1 = children.emplace_back(std::make_unique<BVH_Node>());
        ch1->build(last_aabb, last_rigid_bodies, leaves, lvl + 1, 1);
      }
      else // Vertical split
      {
        AABB<float> first_aabb =
          { aabb.r_min(), aabb.c_min(), aabb.height(), rb_first_mid_aabb.c_max() - aabb.c_min() };
        AABB<float> last_aabb =
          { aabb.r_min(), rb_last_mid_aabb.c_min(), aabb.height(), aabb.c_max() - rb_last_mid_aabb.c_min() };
        
        // Create left and right children and build recursively
        auto& ch0 = children.emplace_back(std::make_unique<BVH_Node>());
        ch0->build(first_aabb, first_rigid_bodies, leaves, lvl + 1, 0);
        
        auto& ch1 = children.emplace_back(std::make_unique<BVH_Node>());
        ch1->build(last_aabb, last_rigid_bodies, leaves, lvl + 1, 1);
      }
    }
    
    AABB<float> refit()
    {
      aabb.set_empty();
      
      if (rigid_body != nullptr)
        aabb = rigid_body->get_curr_AABB();
      else
        for (auto& ch : children)
          aabb.grow(ch->refit());
        
      return aabb;
    }
    
    void find_overlapping_leaves(BVH_Node* leaf, std::vector<BVH_Node*>& overlapping_leaves)
    {
      if (!aabb.overlaps(leaf->aabb))
        return;
      
      if (rigid_body != nullptr && rigid_body != leaf->rigid_body)
        overlapping_leaves.emplace_back(this);
      
      // Recursively check children
      for (auto& ch : children)
        if (ch->aabb.overlaps(leaf->aabb))
          ch->find_overlapping_leaves(leaf, overlapping_leaves);
    }

    
    template<int NR, int NC>
    void draw(ScreenHandler<NR, NC>& sh, int start_level = -1) const
    {
      if (level >= start_level)
      {
        auto rec = aabb.to_rectangle();
        auto color = color::colors_hue_light[static_cast<size_t>(level) % color::colors_hue_light.size()];
        if (order == 1)
          color = color::shade_color(color, color::ShadeType::Dark);
        drawing::draw_box_outline(sh, rec, drawing::OutlineType::Line, { color, Color::Transparent2 });
      }
      for (const auto& ch : children)
        ch->draw(sh);
    }
  };
  
  struct BVHNodePairHash
  {
    std::size_t operator()(const std::pair<BVH_Node*, BVH_Node*>& p) const noexcept
    {
      // Combine the hashes of the two pointers
      return std::hash<BVH_Node*>{}(p.first) ^ (std::hash<BVH_Node*>{}(p.second) << 1);
    }
  };
  
  // ///////////////////////////////////////////
  
  class CollisionHandler
  {
    std::unique_ptr<BVH_Node> m_aabb_bvh;
    std::vector<BVH_Node*> m_aabb_bvh_leaves;
        
  public:
    CollisionHandler()
    {
      m_aabb_bvh = std::make_unique<BVH_Node>();
    }
    
    void rebuild_BVH(int NR, int NC,
                     const DynamicsSystem* dyn_sys)
    {
      m_aabb_bvh->children.clear();
      auto NRf = static_cast<float>(NR);
      auto NCf = static_cast<float>(NC);
      AABB<float> aabb { 0.f, 0.f, NRf, NCf };
      auto rigid_bodies = dyn_sys->get_rigid_bodies_raw();
      m_aabb_bvh_leaves.clear();
      m_aabb_bvh->build(aabb, rigid_bodies, m_aabb_bvh_leaves, 0);
    }
    
    void refit_BVH()
    {
      m_aabb_bvh->refit();
    }
    
    void detect_broad_phase(std::unordered_set<std::pair<BVH_Node*, BVH_Node*>, BVHNodePairHash>& proximity_pairs)
    {
      for (auto* leaf : m_aabb_bvh_leaves)
      {
        std::vector<BVH_Node*> overlapping_leaves;
        m_aabb_bvh->find_overlapping_leaves(leaf, overlapping_leaves);
        
        for (auto* coll_leaf : overlapping_leaves)
        {
          bool order = leaf < coll_leaf;
          BVH_Node* first = order ? leaf : coll_leaf;
          BVH_Node* second = order ? coll_leaf : leaf;
          
          proximity_pairs.insert({first, second});
        }
      }
    }
    
    void detect_narrow_phase(const std::unordered_set<std::pair<BVH_Node*, BVH_Node*>, BVHNodePairHash>& proximity_pairs,
                             std::vector<std::pair<BVH_Node*, BVH_Node*>>& coll_pairs)
    {
      for (const auto& prox_pair : proximity_pairs)
      {
        auto coll_box = prox_pair.first->aabb.set_intersect(prox_pair.first->aabb);
        const auto& aabb_A = prox_pair.first->aabb;
        const auto& aabb_B = prox_pair.second->aabb;
        auto rmin_A = aabb_A.r_min();
        auto cmin_A = aabb_A.c_min();
        auto rmin_B = aabb_B.r_min();
        auto cmin_B = aabb_B.c_min();
        auto rmin = coll_box.r_min();
        auto rmax = coll_box.r_max();
        auto cmin = coll_box.c_min();
        auto cmax = coll_box.c_max();
        const auto& coll_mask_A = prox_pair.first->rigid_body->get_curr_coll_mask();
        const auto& coll_mask_B = prox_pair.second->rigid_body->get_curr_coll_mask();
        for (int r = rmin; r <= rmax; ++r)
        {
          auto r_rel_A = r - rmin_A;
          auto r_rel_B = r - rmin_B;
          for (int c = cmin; c <= cmax; ++c)
          {
            auto c_rel_A = c - cmin_A;
            auto c_rel_B = c - cmin_B;
            auto idx_A = r_rel_A * aabb_A.width() + c_rel_A;
            auto idx_B = r_rel_B * aabb_B.width() + c_rel_B;
            if (coll_mask_A[idx_A] && coll_mask_B[idx_B])
            {
              //std::cout << "Collision!!!" << std::endl;
            }
          }
        }
      }
    }
    
    template<int NR, int NC>
    void draw_BVH(ScreenHandler<NR, NC>& sh, int start_level = -1) const
    {
      m_aabb_bvh->draw(sh, start_level);
    }
    
    void update_detection()
    {
      refit_BVH();
      
      std::unordered_set<std::pair<BVH_Node*, BVH_Node*>, BVHNodePairHash> proximity_pairs;
      detect_broad_phase(proximity_pairs);
      //std::cout << "# coll proximities = " << proximity_pairs.size() << std::endl;
      
      std::vector<std::pair<BVH_Node*, BVH_Node*>> collision_pairs;
      detect_narrow_phase(proximity_pairs, collision_pairs);
    }
    
    
  };
  
}
