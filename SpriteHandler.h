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
#include "AABB.h"
#include <map>
#include <memory>


class Sprite
{
protected:
  std::string name;
  
public:
  RC pos { 0, 0 };
  
  int layer_id = 0; // 0 is the bottom layer.
  bool enabled = true;
  
  std::function<int(int)> func_calc_anim_frame = [](int sim_frame) -> int { return 0; };
  
  virtual ~Sprite() = default;
  Sprite(const std::string& a_name) : name(a_name) {}
  
  virtual AABB<int> calc_curr_AABB(int /*sim_frame*/) = 0;
};

class BitmapSprite : public Sprite
{
  // Helper function for setting any vector data
  template<typename T, typename... Args>
  void set_sprite_data(std::vector<T>& target, Args... args)
  {
    if (sizeof...(args) != static_cast<size_t>(area))
      throw std::invalid_argument("Number of arguments must match sprite size.");
    target = {static_cast<T>(args)...}; // Unpack and assign to the target vector
  }
  
  template<typename T, typename... Args>
  void set_sprite_data(std::vector<T>& target, const ttl::Rectangle& bb, Args... args)
  {
    int nr = std::min(bb.r_len, size.r);
    int nc = std::min(bb.c_len, size.c);
    if (sizeof...(args) != static_cast<size_t>(nr * nc))
      throw std::invalid_argument("Number of arguments must match sprite size and or bounding box size.");
    std::vector<T> source = {static_cast<T>(args)...}; // Unpack and assign to the target vector

    for (int i = 0; i < nr; ++i)
    {
        for (int j = 0; j < nc; ++j)
        {
            // Map bounding box (i, j) to the target sprite data
            int trg_row = bb.r + i;
            int trg_col = bb.c + j;

            // Calculate linear index into the target vector and src_data
            int trg_index = trg_row * size.c + trg_col;
            int src_index = i * nc + j;

            // Assign data to the target vector
            target[trg_index] = source[src_index];
        }
    }
  }

  RC size { 0, 0 };
  int area = 0;
  std::vector<std::unique_ptr<drawing::Texture>> texture_frames;
  
  drawing::Texture* fetch_frame(int anim_frame)
  {
    while (texture_frames.size() <= anim_frame)
      texture_frames.emplace_back(std::make_unique<drawing::Texture>());
    return texture_frames[anim_frame].get();
  }

public:
  BitmapSprite(const std::string& a_name) : Sprite(a_name) {}
  
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
      else
        std::cout << "ERROR in clone_frame() : anim_frame must be larger than or equal to the number of texture frames!" << std::endl;
    }
    else
      std::cout << "ERROR in clone_frame() : from_anim_frame cannot be larger than or equal to the number of texture frames!" << std::endl;
  }
  
  // #FIXME: Perhaps move these varyadic functions to Texture for more versatility.
  
  // Set sprite characters
  template<typename... Chars>
  void set_sprite_chars(int anim_frame, Chars... ch)
  {
    auto* texture = fetch_frame(anim_frame);
    set_sprite_data(texture->characters, ch...);
  }
  
  template<typename... Chars>
  void set_sprite_chars(int anim_frame, const ttl::Rectangle& bb, Chars... ch)
  {
    auto* texture = fetch_frame(anim_frame);
    set_sprite_data(texture->characters, bb, ch...);
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
  
  // Set sprite foreground colors
  template<typename... Colors>
  void set_sprite_fg_colors(int anim_frame, const ttl::Rectangle& bb, Colors... fg_color)
  {
    auto* texture = fetch_frame(anim_frame);
    set_sprite_data(texture->fg_colors, bb, fg_color...);
  }
  
  // Set sprite background colors
  template<typename... Colors>
  void set_sprite_bg_colors(int anim_frame, Colors... bg_color)
  {
    auto* texture = fetch_frame(anim_frame);
    set_sprite_data(texture->bg_colors, bg_color...);
  }
  
  // Set sprite background colors
  template<typename... Colors>
  void set_sprite_bg_colors(int anim_frame, const ttl::Rectangle& bb, Colors... bg_color)
  {
    auto* texture = fetch_frame(anim_frame);
    set_sprite_data(texture->bg_colors, bb, bg_color...);
  }
  
  // Set sprite materials
  template<typename... Materials>
  void set_sprite_materials(int anim_frame, Materials... mat)
  {
    auto* texture = fetch_frame(anim_frame);
    set_sprite_data(texture->materials, mat...);
  }
  
  // Set sprite materials
  template<typename... Materials>
  void set_sprite_materials(int anim_frame, const ttl::Rectangle& bb, Materials... mat)
  {
    auto* texture = fetch_frame(anim_frame);
    set_sprite_data(texture->materials, bb, mat...);
  }
  
  const drawing::Texture& get_curr_frame(int sim_frame)
  {
    int frame_id = func_calc_anim_frame(sim_frame);
    if (frame_id >= texture_frames.size())
      throw std::invalid_argument("ERROR: Incorrect frame id: " + std::to_string(frame_id) + " for sprite \"" + name + "\"! Sprite only has " + std::to_string(texture_frames.size()) + " frames.");
    return *texture_frames[frame_id];
  }
  
  template<int NR, int NC>
  void draw(ScreenHandler<NR, NC>& sh, int sim_frame)
  {
    auto& texture = get_curr_frame(sim_frame);
    
    drawing::draw_box_textured(sh,
                               pos.r - 1, pos.c - 1,
                               texture.size.r + 2, texture.size.c + 2,
                               drawing::SolarDirection::Zenith,
                               texture);
    
  }
  
  virtual AABB<int> calc_curr_AABB(int /*sim_frame*/) override
  {
    return { pos.r, pos.c, size.r, size.c };
  }
};

class VectorSprite : public Sprite
{
  struct LineSeg
  {
    std::array<RC, 2> pos;
    char ch = 0;
    styles::Style style;
    int mat = 0;
  };
  
  struct VectorFrame
  {
    std::vector<LineSeg> line_segments;
  };
  
  std::vector<std::unique_ptr<VectorFrame>> vector_frames;
  
  float rot_rad = 0.f;
  
  VectorFrame* fetch_frame(int anim_frame)
  {
    while (vector_frames.size() <= anim_frame)
      vector_frames.emplace_back(std::make_unique<VectorFrame>());
    return vector_frames[anim_frame].get();
  }
  
  std::pair<RC, RC> calc_seg_world_pos(const LineSeg& line_seg) const
  {
    const auto aspect_ratio = 1.5f;
    auto rr0 = static_cast<float>(line_seg.pos[0].r);
    auto cc0 = static_cast<float>(line_seg.pos[0].c);
    auto rr1 = static_cast<float>(line_seg.pos[1].r);
    auto cc1 = static_cast<float>(line_seg.pos[1].c);
    float C = std::cos(rot_rad);
    float S = std::sin(rot_rad);
    auto r0 = pos.r + math::roundI(C*rr0 - S*cc0);
    auto c0 = pos.c + math::roundI((S*rr0 + C*cc0)*aspect_ratio);
    auto r1 = pos.r + math::roundI(C*rr1 - S*cc1);
    auto c1 = pos.c + math::roundI((S*rr1 + C*cc1)*aspect_ratio);
    RC p0 { r0, c0 };
    RC p1 { r1, c1 };
    return { p0, p1 };
  }
  
public:
  VectorSprite(const std::string& a_name) : Sprite(a_name) {}
  
  void add_line_segment(int anim_frame, const RC& p0, const RC& p1, char ch, const styles::Style& style, int mat = 0)
  {
    auto* vector_frame = fetch_frame(anim_frame);
    auto& line_seg = vector_frame->line_segments.emplace_back();
    line_seg.pos[0] = p0;
    line_seg.pos[1] = p1;
    line_seg.ch = ch;
    line_seg.style = style;
    line_seg.mat = mat;
  }
  
  const VectorFrame& get_curr_frame(int sim_frame)
  {
    int frame_id = func_calc_anim_frame(sim_frame);
    if (frame_id >= vector_frames.size())
      throw std::invalid_argument("ERROR: Incorrect frame id: " + std::to_string(frame_id) + " for sprite \"" + name + "\"! Sprite only has " + std::to_string(vector_frames.size()) + " frames.");
    return *vector_frames[frame_id];
  }
  
  void set_rotation(float rot_deg)
  {
    rot_rad = math::deg2rad(rot_deg);
  }
  
  template<int NR, int NC>
  void draw(ScreenHandler<NR, NC>& sh, int sim_frame)
  {
    auto& vector_frame = get_curr_frame(sim_frame);
    
    for (const auto& line_seg : vector_frame.line_segments)
    {
      auto [p0, p1] = calc_seg_world_pos(line_seg);
      bresenham::plot_line(sh, p0, p1, std::string(1, line_seg.ch), line_seg.style.fg_color, line_seg.style.bg_color);
    }
  }
  
  virtual AABB<int> calc_curr_AABB(int sim_frame) override
  {
    auto& vector_frame = get_curr_frame(sim_frame);

    AABB<int> aabb;
    for (const auto& line_seg : vector_frame.line_segments)
    {
      auto [p0, p1] = calc_seg_world_pos(line_seg);
      aabb.add_point(p0);
      aabb.add_point(p1);
    }
    return aabb;
  }
};

// /////////////////////////////////////

class SpriteHandler
{
  std::map<std::string, std::unique_ptr<Sprite>> m_sprites;

public:
  SpriteHandler() = default;
  ~SpriteHandler() = default;
  
  BitmapSprite* create_bitmap_sprite(const std::string& sprite_name)
  {
    m_sprites[sprite_name] = std::make_unique<BitmapSprite>(sprite_name);
    // We simply assume that it was successfully created.
    return static_cast<BitmapSprite*>(m_sprites[sprite_name].get());
  }
  
  VectorSprite* create_vector_sprite(const std::string& sprite_name)
  {
    m_sprites[sprite_name] = std::make_unique<VectorSprite>(sprite_name);
    // We simply assume that it was successfully created.
    return static_cast<VectorSprite*>(m_sprites[sprite_name].get());
  }
  
  Sprite* fetch_sprite(const std::string& sprite_name)
  {
    auto it = m_sprites.find(sprite_name);
    if (it != m_sprites.end())
      return it->second.get();
    return nullptr;
  }
  
  template<int NR, int NC>
  void draw(ScreenHandler<NR, NC>& sh, int sim_frame) const
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
          if (auto* bitmap_sprite = dynamic_cast<BitmapSprite*>(sprite.get()); bitmap_sprite != nullptr)
            bitmap_sprite->draw(sh, sim_frame);
          else if (auto* vector_sprite = dynamic_cast<VectorSprite*>(sprite.get()); vector_sprite != nullptr)
            vector_sprite->draw(sh, sim_frame);
        }
      }
    }
  }
  
  template<int NR, int NC>
  void draw_dbg(ScreenHandler<NR, NC>& sh, int sim_frame) const
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
          AABB<int> aabb;
          if (auto* bitmap_sprite = dynamic_cast<BitmapSprite*>(sprite.get()); bitmap_sprite != nullptr)
            aabb = bitmap_sprite->calc_curr_AABB(sim_frame);
          else if (auto* vector_sprite = dynamic_cast<VectorSprite*>(sprite.get()); vector_sprite != nullptr)
            aabb = vector_sprite->calc_curr_AABB(sim_frame);
            
          auto rec = aabb.to_rectangle();
          drawing::draw_box_outline(sh, rec, drawing::OutlineType::Line, { Color::LightGray, Color::Transparent2 });
        }
      }
    }
  }
};
