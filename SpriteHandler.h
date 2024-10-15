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
    const size_t sprite_size = texture.area;
    if (sizeof...(args) != sprite_size)
      throw std::invalid_argument("Number of arguments must match sprite size.");
    target = {args...}; // Unpack and assign to the target vector
  }

public:
  RC pos { 0, 0 };
  int layer_id = 0; // 0 is the bottom layer.
  bool enabled = true;
  drawing::Texture texture;
  
  // Initialize the sprite's dimensions (NR and NC)
  void init(int NR, int NC)
  {
    texture.clear();
    texture.area = NR * NC;
    texture.size = { NR, NC };
    texture.characters.resize(NR * NC);
    texture.fg_colors.resize(NR * NC);
    texture.bg_colors.resize(NR * NC);
    texture.materials.resize(NR * NC);
  }
  
  // Set sprite characters
  template<typename... Chars>
  void set_sprite_chars(Chars... ch)
  {
    set_sprite_data(texture.characters, ch...);
  }
  
  // Set sprite foreground colors
  template<typename... Colors>
  void set_sprite_fg_colors(Colors... fg_color)
  {
    set_sprite_data(texture.fg_colors, fg_color...);
  }
  
  // Set sprite background colors
  template<typename... Colors>
  void set_sprite_bg_colors(Colors... bg_color)
  {
    set_sprite_data(texture.bg_colors, bg_color...);
  }
  
  // Set sprite materials
  template<typename... Materials>
  void set_sprite_materials(Materials... mat)
  {
    set_sprite_data(texture.materials, mat...);
  }
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
  void draw(ScreenHandler<NR, NC>& sh) const
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
          drawing::draw_box_textured(sh,
                          sprite->pos.r - 1, sprite->pos.c - 1,
                          sprite->texture.size.r + 2, sprite->texture.size.c + 2,
                          drawing::SolarDirection::Zenith,
                          sprite->texture);
      }
    }
  }
};
