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
    target = {static_cast<T>(args)...}; // Unpack and assign to the target vector
  }
  
  RC size { 0, 0 };
  int area = 0;
  std::vector<std::unique_ptr<drawing::Texture>> texture_frames;
  std::string name;
  
  drawing::Texture* fetch_frame(int anim_frame)
  {
    while (texture_frames.size() <= anim_frame)
      texture_frames.emplace_back(std::make_unique<drawing::Texture>());
    return texture_frames[anim_frame].get();
  }

public:
  RC pos { 0, 0 };
  
  int layer_id = 0; // 0 is the bottom layer.
  bool enabled = true;
  
  Sprite(const std::string& a_name) : name(a_name) {}
  
  // Initialize the sprite's dimensions (NR and NC)
  void init(int NR, int NC)
  {
    size = { NR, NC };
    area = NR * NC;
  }
  
  void create_frame(int anim_frame)
  {
    auto* texture = fetch_frame(anim_frame);
    texture->clear();
    texture->area = area;
    texture->size = size;
    texture->characters.resize(area);
    texture->fg_colors.resize(area);
    texture->bg_colors.resize(area);
    texture->materials.resize(area);
  }
  
  void load_frame(int anim_frame, const std::string& file_path)
  {
    auto* texture = fetch_frame(anim_frame);
    texture->clear();
    texture->load(file_path);
    if (texture->size != size)
      throw std::invalid_argument("Loaded sprite frame doesn't have the same size as the sprite itself.");
  }
  
  void save_frame(int anim_frame, const std::string& file_path)
  {
    auto* texture = fetch_frame(anim_frame);
    texture->save(file_path);
  }
  
  void clone_frame(int anim_frame, int from_anim_frame)
  {
    const auto N = static_cast<int>(texture_frames.size());
    if (from_anim_frame < N)
    {
      if (anim_frame >= N)
      {
        auto* texture_from = fetch_frame(from_anim_frame);
        fetch_frame(anim_frame);
        texture_frames[anim_frame] = std::make_unique<drawing::Texture>(*texture_from);
      }
    }
  }
  
  // #FIXME: Perhaps move these varyadic functions to Texture for more versatility.
  
  // Set sprite characters
  template<typename... Chars>
  void set_sprite_chars(int anim_frame, Chars... ch)
  {
    auto* texture = fetch_frame(anim_frame);
    set_sprite_data(texture->characters, ch...);
  }
  
  // Set sprite characters from a string for each row
  template<typename... Strings>
  void set_sprite_chars_from_strings(int anim_frame, Strings... rows)
  {
    auto* texture = fetch_frame(anim_frame);
    
    std::array<std::string, sizeof...(rows)> row_array = { rows... };
    
    // Check that the number of rows matches the texture's height
    if (row_array.size() != texture->size.r)
      throw std::invalid_argument("Number of strings must match the number of rows.");
    
    for (const auto& row : row_array)
      if (row.size() != texture->size.c)
        throw std::invalid_argument("Each string must have exactly NC characters.");
    
    // Unpack strings into the characters vector
    int idx = 0;
    for (const auto& row : row_array)
      for (char ch : row)
        texture->characters[idx++] = ch;
  }
  
  // Set sprite foreground colors
  template<typename... Colors>
  void set_sprite_fg_colors(int anim_frame, Colors... fg_color)
  {
    auto* texture = fetch_frame(anim_frame);
    set_sprite_data(texture->fg_colors, fg_color...);
  }
  
  // Set sprite background colors
  template<typename... Colors>
  void set_sprite_bg_colors(int anim_frame, Colors... bg_color)
  {
    auto* texture = fetch_frame(anim_frame);
    set_sprite_data(texture->bg_colors, bg_color...);
  }
  
  // Set sprite materials
  template<typename... Materials>
  void set_sprite_materials(int anim_frame, Materials... mat)
  {
    auto* texture = fetch_frame(anim_frame);
    set_sprite_data(texture->materials, mat...);
  }
  
  std::function<int(int)> func_frame_to_texture = [](int anim_frame) -> int { return 0; };
  
  const drawing::Texture& get_curr_frame_texture(int anim_frame)
  {
    int tex_id = func_frame_to_texture(anim_frame);
    if (tex_id >= texture_frames.size())
      throw std::invalid_argument("ERROR: Incorrect frame id: " + std::to_string(tex_id) + " for sprite \"" + name + "\"! Sprite only has " + std::to_string(texture_frames.size()) + " frames.");
    return *texture_frames[tex_id];
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
    m_sprites[sprite_name] = std::make_unique<Sprite>(sprite_name);
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
          auto& texture = sprite->get_curr_frame_texture(anim_frame);
          
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
