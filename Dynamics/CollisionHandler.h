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
#include <Core/Utils.h>
#include <unordered_set>

using namespace utils::literals;


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
  public:
    struct IsectData
    {
      Vec2 world_pos;
      BVH_Node* node_A = nullptr;
      BVH_Node* node_B = nullptr;
    };
    
  private:
    std::unique_ptr<BVH_Node> m_aabb_bvh;
    std::vector<BVH_Node*> m_aabb_bvh_leaves;
    std::vector<std::pair<RigidBody*, RigidBody*>> exclusion_pairs;
    std::vector<std::pair<std::string, std::string>> exclusion_prefixes;
    
    struct NarrowPhaseCollData
    {
      BVH_Node* node_A = nullptr;
      BVH_Node* node_B = nullptr;
      std::vector<Vec2> local_pos_A, local_pos_B;
    };
    
    std::vector<IsectData> isect_world_positions;
        
  public:
    CollisionHandler()
    {
      m_aabb_bvh = std::make_unique<BVH_Node>();
    }
    
    const std::vector<IsectData>& get_isect_world_positions() const
    {
      return isect_world_positions;
    }
    
    void exclude_rigid_body_pairs(RigidBody* rb_A, RigidBody* rb_B)
    {
      if (rb_A > rb_B)
        std::swap(rb_A, rb_B);
        
      if (!stlutils::contains_if(exclusion_pairs,
            [rb_A, rb_B](const auto& rbp) { return rbp.first == rb_A && rbp.second == rb_B; }))
      {
        exclusion_pairs.emplace_back(rb_A, rb_B);
      }
    }
    
    void reinclude_rigid_body_pairs(RigidBody* rb_A, RigidBody* rb_B)
    {
      if (rb_A > rb_B)
        std::swap(rb_A, rb_B);
        
      stlutils::erase_if(exclusion_pairs,
            [rb_A, rb_B](const auto& rbp) { return rbp.first == rb_A && rbp.second == rb_B; });
    }
    
    void exclude_all_rigid_bodies_of_prefixes(const DynamicsSystem* dyn_sys,
                                              std::string sprite_prefix_A,
                                              std::string sprite_prefix_B,
                                              bool enforce_prefixes = false)
    {
      auto rb_vec = dyn_sys->get_rigid_bodies_raw();
      
      const auto num_A = stlutils::count_if(rb_vec, [sprite_prefix_A](const auto* rb)
      {
        return rb->get_sprite()->get_name().starts_with(sprite_prefix_A);
      });
      const auto num_B = stlutils::count_if(rb_vec, [sprite_prefix_B](const auto* rb)
      {
        return rb->get_sprite()->get_name().starts_with(sprite_prefix_B);
      });
      
      if (num_A * num_B > 1e4_i || enforce_prefixes)
      {
        if (sprite_prefix_A > sprite_prefix_B)
          std::swap(sprite_prefix_A, sprite_prefix_B);
        
        if (!stlutils::contains_if(exclusion_prefixes,
              [sprite_prefix_A, sprite_prefix_B](const auto& strp)
              { return strp.first == sprite_prefix_A && strp.second == sprite_prefix_B; }))
        {
          exclusion_prefixes.emplace_back(sprite_prefix_A, sprite_prefix_B);
        }
      }
      else
      {
        for (auto* rb_A : rb_vec)
        {
          auto* sprite_A = rb_A->get_sprite();
          if (sprite_A == nullptr || !sprite_A->get_name().starts_with(sprite_prefix_A))
            continue;
          for (auto* rb_B : rb_vec)
          {
            if (rb_B == rb_A)
              continue;
        
            auto* sprite_B = rb_B->get_sprite();
            if (sprite_B == nullptr || !sprite_B->get_name().starts_with(sprite_prefix_B))
              continue;
        
            exclude_rigid_body_pairs(rb_A, rb_B);
          }
        }
      }
    }
    
    void reinclude_all_rigid_bodies_of_prefixes(const DynamicsSystem* dyn_sys,
                                                std::string sprite_prefix_A,
                                                std::string sprite_prefix_B)
    {
      auto rb_vec = dyn_sys->get_rigid_bodies_raw();
      
      if (sprite_prefix_A > sprite_prefix_B)
        std::swap(sprite_prefix_A, sprite_prefix_B);
        
      stlutils::erase_if(exclusion_prefixes,
                         [sprite_prefix_A, sprite_prefix_B](const auto& strp)
                         { return strp.first == sprite_prefix_A && strp.second == sprite_prefix_B; });
      
      for (auto* rb_A : rb_vec)
      {
        auto* sprite_A = rb_A->get_sprite();
        if (sprite_A == nullptr || !sprite_A->get_name().starts_with(sprite_prefix_A))
          continue;
        for (auto* rb_B : rb_vec)
        {
          if (rb_B == rb_A)
            continue;
      
          auto* sprite_B = rb_B->get_sprite();
          if (sprite_B == nullptr || !sprite_B->get_name().starts_with(sprite_prefix_B))
            continue;
      
          reinclude_rigid_body_pairs(rb_A, rb_B);
        }
      }
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
          
          auto* rb_A = first->rigid_body;
          auto* rb_B = second->rigid_body;
          if (rb_A > rb_B)
            std::swap(rb_A, rb_B);
            
          if (stlutils::contains_if(exclusion_pairs,
            [rb_A, rb_B](const auto& rbp) { return rbp.first == rb_A && rbp.second == rb_B; }))
          {
            continue;
          }
          const auto& name_A = rb_A->get_sprite()->get_name();
          const auto& name_B = rb_B->get_sprite()->get_name();
          if (stlutils::contains_if(exclusion_prefixes,
            [&name_A, &name_B](const auto& strp)
            {
              if (name_A.starts_with(strp.first) && name_B.starts_with(strp.second))
                return true;
              if (name_B.starts_with(strp.first) && name_A.starts_with(strp.second))
                return true;
              return false;
            }))
          {
            continue;
          }
          
          proximity_pairs.insert({first, second});
        }
      }
    }
    
    void detect_narrow_phase(const std::unordered_set<std::pair<BVH_Node*, BVH_Node*>, BVHNodePairHash>& proximity_pairs,
                             std::vector<NarrowPhaseCollData>& coll_data)
    {
      coll_data.reserve(proximity_pairs.size());
      for (const auto& prox_pair : proximity_pairs)
      {
        const auto& aabb_A = prox_pair.first->aabb;
        const auto& aabb_B = prox_pair.second->aabb;
        const auto width_A = math::roundI(aabb_A.width());
        const auto width_B = math::roundI(aabb_B.width());
        auto coll_box = aabb_A.set_intersect(aabb_B);
        auto rmin_A = math::roundI(aabb_A.r_min());
        auto cmin_A = math::roundI(aabb_A.c_min());
        auto rmin_B = math::roundI(aabb_B.r_min());
        auto cmin_B = math::roundI(aabb_B.c_min());
        auto rmin = math::roundI(coll_box.r_min());
        auto rmax = math::roundI(coll_box.r_max());
        auto cmin = math::roundI(coll_box.c_min());
        auto cmax = math::roundI(coll_box.c_max());
        const auto& coll_mask_A = prox_pair.first->rigid_body->get_curr_coll_mask();
        const auto& coll_mask_B = prox_pair.second->rigid_body->get_curr_coll_mask();
        NarrowPhaseCollData cdata;
        for (int r = rmin; r < rmax; ++r)
        {
          auto r_rel_A = r - rmin_A;
          auto r_rel_B = r - rmin_B;
          for (int c = cmin; c < cmax; ++c)
          {
            auto c_rel_A = c - cmin_A;
            auto c_rel_B = c - cmin_B;
            auto idx_A = r_rel_A * width_A + c_rel_A;
            auto idx_B = r_rel_B * width_B + c_rel_B;
            if (coll_mask_A[idx_A] && coll_mask_B[idx_B])
            {
              //std::cout << "Collision!!!" << std::endl;
              cdata.node_A = prox_pair.first;
              cdata.node_B = prox_pair.second;
              cdata.local_pos_A.emplace_back(static_cast<float>(r_rel_A), static_cast<float>(c_rel_A));
              cdata.local_pos_B.emplace_back(static_cast<float>(r_rel_B), static_cast<float>(c_rel_B));
            }
          }
        }
        if (cdata.node_A != nullptr && cdata.node_B != nullptr)
          coll_data.emplace_back(cdata);
      }
    }
    
    template<int NR, int NC>
    void draw_dbg_broad_phase(ScreenHandler<NR, NC>& sh, int start_level = -1) const
    {
      m_aabb_bvh->draw(sh, start_level);
    }
    
    template<int NR, int NC>
    void draw_dbg_narrow_phase(ScreenHandler<NR, NC>& sh, Color coll_fg_color = Color::Magenta) const
    {
      for (const auto& pt : isect_world_positions)
      {
        auto rc = to_RC_round(pt.world_pos);
        sh.write_buffer("X", rc.r, rc.c, coll_fg_color);
      }
    }
    
    void update_detection(std::vector<NarrowPhaseCollData>& collision_data, bool verbose = false)
    {
      refit_BVH();
      
      std::unordered_set<std::pair<BVH_Node*, BVH_Node*>, BVHNodePairHash> proximity_pairs;
      detect_broad_phase(proximity_pairs);
      if (verbose)
        std::cout << "# coll proximities = " << proximity_pairs.size() << std::endl;
      
      detect_narrow_phase(proximity_pairs, collision_data);
      
      isect_world_positions.clear(); // For debug drawing.
      for (const auto& cd : collision_data)
      {
        auto* node_A = cd.node_A;
        const auto& aabb_A = node_A->aabb;
      
        size_t num_pts = cd.local_pos_A.size();
        for (size_t pt_idx = 0; pt_idx < num_pts; ++pt_idx)
        {
          auto contact_local_A = cd.local_pos_A[pt_idx];
          auto contact_world_A = Vec2 { aabb_A.r_min(), aabb_A.c_min() } + contact_local_A;
          isect_world_positions.emplace_back(IsectData { contact_world_A, cd.node_A, cd.node_B });
        }
      }
      if (verbose)
        std::cout << "# collisions = " << collision_data.size() << std::endl;
    }
    
    void update_response(const std::vector<NarrowPhaseCollData>& collision_data)
    {
      for (const auto& cd : collision_data)
      {
        auto* node_A = cd.node_A;
        auto* node_B = cd.node_B;
        auto* rb_A = node_A->rigid_body;
        auto* rb_B = node_B->rigid_body;
        // Nothing can happen if both rbs are sleeping.
        if (rb_A->is_sleeping() && rb_B->is_sleeping())
          continue;
        const auto& aabb_A = node_A->aabb;
        const auto& aabb_B = node_B->aabb;
        auto e_A = rb_A->get_e();
        auto e_B = rb_B->get_e();
        auto dyn_friction_A = rb_A->get_dynamic_friction();
        auto dyn_friction_B = rb_B->get_dynamic_friction();
        auto friction = std::sqrt(dyn_friction_A * dyn_friction_B);
      
        size_t num_pts = cd.local_pos_A.size();
        for (size_t pt_idx = 0; pt_idx < num_pts; ++pt_idx)
        {
          auto contact_local_A = cd.local_pos_A[pt_idx];
          auto contact_local_B = cd.local_pos_B[pt_idx];
          
          auto contact_world_A = Vec2 { aabb_A.r_min(), aabb_A.c_min() } + contact_local_A;
          auto contact_world_B = Vec2 { aabb_B.r_min(), aabb_B.c_min() } + contact_local_B;
          assert(contact_world_A.r == contact_world_B.r);
          assert(contact_world_A.c == contact_world_B.c);
          
          // Calculate relative velocity at the contact point.
          auto vel_A = rb_A->calc_velocity_at(contact_world_A);
          auto vel_B = rb_B->calc_velocity_at(contact_world_B);
          Vec2 relative_velocity = vel_B - vel_A;
          
          auto normal_A = rb_A->fetch_surface_normal(to_RC_round(contact_local_A));
          auto normal_B = rb_B->fetch_surface_normal(to_RC_round(contact_local_B));
          auto collision_normal = math::normalize(normal_A - normal_B);
          
          // Calculate relative velocity in the direction of the normal.
          auto velocity_along_normal = math::dot(relative_velocity, collision_normal);
          
          // Skip if objects are separating.
          if (velocity_along_normal > 0.f) continue;
          
          // Compute impulse scalar
          auto j_num = velocity_along_normal;
          auto j_den = rb_A->get_inv_mass() + rb_B->get_inv_mass() +
          rb_A->get_inv_Iz() * math::sq(math::dot(contact_world_A - rb_A->get_curr_cm(), collision_normal)) +
          rb_B->get_inv_Iz() * math::sq(math::dot(contact_world_B - rb_B->get_curr_cm(), collision_normal));
          float j = j_num / j_den;
          
          // Calculate impulse vector.
          auto impulse = collision_normal * j;
          
          // Apply impulse to both objects.
          rb_A->apply_impulse(+(1.f + e_A) * impulse, contact_world_A);
          rb_B->apply_impulse(-(1.f + e_B) * impulse, contact_world_B);
          
          // Calculate tangential (frictional) component of relative velocity
          Vec2 tangential_velocity = relative_velocity - collision_normal * velocity_along_normal;
          auto tang_vel_magnitude = math::length(tangential_velocity);
          if (tang_vel_magnitude > 0.f) // Only apply friction if there’s tangential movement
          {
            // Compute friction impulse magnitude
            Vec2 friction_impulse = friction * tangential_velocity / j_den;
            
            // Apply friction impulse
            rb_A->apply_impulse(friction_impulse, contact_world_A);
            rb_B->apply_impulse(-friction_impulse, contact_world_B);
          }
        }
      }
    }
    
    void update(bool verbose = false)
    {
      std::vector<NarrowPhaseCollData> narrow_phase_collision_data;
      update_detection(narrow_phase_collision_data, verbose);
      
      update_response(narrow_phase_collision_data);
    }
    
    
  };
  
}
