//
//  SpriteHandler.h
//  Termin8or
//
//  Created by Rasmus Anthin on 2024-10-14.
//

#pragma once
#include "Texture.h"
#include "ScreenHandler.h"
#include "Drawing.h"
#include <map>
#include <memory>


class Sprite
{
  // Helper function for setting any vector data
  template<typename T, typename... Args>
  void set_sprite_data(std::vector<T>& target, Args... args)
  {
    const size_t sprite_size = area;
    if (sizeof...(args) != sprite_size)
      throw std::invalid_argument("Number of arguments must match sprite size.");
    target = {args...}; // Unpack and assign to the target vector
  }

public:
  RC pos { 0, 0 };
  RC size { 0, 0 };
  int area = 0;
  
  int layer_id = 0; // 0 is the bottom layer.
  bool enabled = true;
  
  std::vector<drawing::Texture> texture_frames;
  
  // Initialize the sprite's dimensions (NR and NC)
  void init(int NR, int NC)
  {
    size = { NR, NC };
    area = NR * NC;
  }
  
  void init_frame(int anim_frame)
  {
    auto& texture = stlutils::at_growing(texture_frames, anim_frame);
    texture.clear();
    texture.area = area;
    texture.size = size;
    texture.characters.resize(area);
    texture.fg_colors.resize(area);
    texture.bg_colors.resize(area);
    texture.materials.resize(area);
  }
  
  // Set sprite characters
  template<typename... Chars>
  void set_sprite_chars(int anim_frame, Chars... ch)
  {
    auto& texture = stlutils::at_growing(texture_frames, anim_frame);
    set_sprite_data(texture.characters, ch...);
  }
  
  // Set sprite foreground colors
  template<typename... Colors>
  void set_sprite_fg_colors(int anim_frame, Colors... fg_color)
  {
    auto& texture = stlutils::at_growing(texture_frames, anim_frame);
    set_sprite_data(texture.fg_colors, fg_color...);
  }
  
  // Set sprite background colors
  template<typename... Colors>
  void set_sprite_bg_colors(int anim_frame, Colors... bg_color)
  {
    auto& texture = stlutils::at_growing(texture_frames, anim_frame);
    set_sprite_data(texture.bg_colors, bg_color...);
  }
  
  // Set sprite materials
  template<typename... Materials>
  void set_sprite_materials(int anim_frame, Materials... mat)
  {
    auto& texture = stlutils::at_growing(texture_frames, anim_frame);
    set_sprite_data(texture.materials, mat...);
  }
  
  std::function<int(int)> func_frame_to_texture = [](int anim_frame) -> int { return 0; };
};

// /////////////////////////////////////

class SpriteHandler
{
  std::map<std::string, std::unique_ptr<Sprite>> m_sprites;

public:
  SpriteHandler() = default;
  ~SpriteHandler() = default;
  
  Sprite* create_sprite(const std::string& sprite_name)
  {
    m_sprites[sprite_name] = std::make_unique<Sprite>();
    // We simply assume that it was successfully created.
    return m_sprites[sprite_name].get();
  }
  
  Sprite* fetch_sprite(const std::string& sprite_name)
  {
    auto it = m_sprites.find(sprite_name);
    if (it != m_sprites.end())
      return it->second.get();
    return nullptr;
  }
  
  template<int NR, int NC>
  void draw(ScreenHandler<NR, NC>& sh, int anim_frame) const
  {
    int max_layer_id = 0;
    for (const auto& sprite_pair : m_sprites)
      if (sprite_pair.second->enabled)
        math::maximize(max_layer_id, sprite_pair.second->layer_id);
      
    for (int layer_id = max_layer_id; layer_id >= 0; --layer_id)
    {
      for (const auto& sprite_pair : m_sprites)
      {
        const auto& sprite = sprite_pair.second;
        if (sprite->enabled && sprite->layer_id == layer_id)
        {
          int tex_id = sprite->func_frame_to_texture(anim_frame);
          auto& texture = stlutils::at_growing(sprite->texture_frames, tex_id);
        
          drawing::draw_box_textured(sh,
                          sprite->pos.r - 1, sprite->pos.c - 1,
                          texture.size.r + 2, texture.size.c + 2,
                          drawing::SolarDirection::Zenith,
                          texture);
        }
      }
    }
  }
};
