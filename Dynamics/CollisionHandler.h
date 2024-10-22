//
//  CollisionHandler.h
//  Termin8or
//
//  Created by Rasmus Anthin on 2024-10-21.
//

#pragma once
#include "../Rectangle.h"

namespace dynamics
{

  struct BVH_Node
  {
    ttl::Rectangle aabb;
    RC centroid;
    Sprite* sprite = nullptr;
    std::vector<std::unique_ptr<BVH_Node>> children;
  };
  
  //struct QuadTree
  //{
  //  ttl::Rectangle bb;
  //  std::vector<Sprite*> sprites;
  //  std::array<QuadTree*, 4> children { nullptr, nullptr, nullptr, nullptr };
  //
  //  QuadTree(const ttl::Rectangle& bounding_box)
  //  : bb(bounding_box)
  //  {}
  //};
  
  class CollisionHandler
  {
    
    //struct SpriteSpriteCollData
    //{
    //  SpriteSpriteCollData(bool a_enabled, Sprite* a_spriteA, Sprite* a_spriteB)
    //    : enabled(a_enabled), spriteA(a_spriteA), spriteB(a_spriteB)
    //  {}
    //
    //  bool enabled = true;
    //  Sprite* spriteA = nullptr;
    //  Sprite* spriteB = nullptr;
    //};
    
    //std::vector<SpriteSpriteCollData> m_sprite_coll_pairs;
    
    std::unique_ptr<BVH_Node> m_aabb_bvh;
    
    std::vector<Sprite*> m_coll_sprites;
    
  public:
    CollisionHandler()
    {
      m_aabb_bvh = std::make_unique<BVH_Node>();
    }
    
    void add_coll_sprites(Sprite* sprite)
    {
      m_coll_sprites.emplace_back(sprite);
    }
    
    void rebuild_AABB_bvh()
    {
      
    }
    
    //void register_sprite_collision_pairs(Sprite* spriteA, Sprite* spriteB, bool enabled)
    //{
    //  m_sprite_coll_pairs.emplace_back(enabled, spriteA, spriteB);
    //}
    
    void update_detection()
    {
      
    }
    
    
  };
  
}
