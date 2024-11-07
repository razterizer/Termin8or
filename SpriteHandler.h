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
#include <Core/Vec2.h>
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
  
  const std::string& get_name() const { return name; }
  
  virtual void clone_frame(int anim_frame, int from_anim_frame) = 0;
  
  virtual AABB<int> calc_curr_AABB(int /*sim_frame*/) const = 0;
  
  virtual Vec2 calc_curr_centroid(int /*sim_frame*/) const = 0;
  
  virtual int num_frames() const = 0;
  
  virtual bool_vector calc_curr_coll_mask(int sim_frame, int collision_material) = 0;
  
  virtual Vec2 get_pos_offs(int sim_frame) const = 0;
};

// /////////////////////////////////////////////////

class BitmapSprite : public Sprite
{
  // Helper function for setting any vector data
  template<typename T, typename... Args>
  bool set_sprite_data(std::vector<T>& target, Args... args)
  {
    if (sizeof...(args) != static_cast<size_t>(area))
    {
      std::cerr << "ERROR in BitmapSprite::set_sprite_data() : Number of arguments must match sprite size." << std::endl;
      return false;
    }
    target = {static_cast<T>(args)...}; // Unpack and assign to the target vector
    return true;
  }
  
  template<typename T, typename... Args>
  bool set_sprite_data(std::vector<T>& target, const ttl::Rectangle& bb, Args... args)
  {
    int nr = std::min(bb.r_len, size.r);
    int nc = std::min(bb.c_len, size.c);
    if (sizeof...(args) != static_cast<size_t>(nr * nc))
    {
      std::cerr << "ERROR in BitmapSprite::set_sprite_data() : Number of arguments must match sprite size and or bounding box size." << std::endl;
      return false;
    }
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
    
    return true;
  }

  RC size { 0, 0 };
  int area = 0;
  std::vector<std::unique_ptr<drawing::Texture>> texture_frames;
  
  drawing::Texture* fetch_frame(int anim_frame)
  {
    while (stlutils::sizeI(texture_frames) <= anim_frame)
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
  
  RC get_size() const
  {
    return size;
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
  
  bool load_frame(int anim_frame, const std::string& file_path)
  {
    auto* texture = fetch_frame(anim_frame);
    texture->clear();
    texture->load(file_path);
    if (texture->size != size)
    {
      std::cerr << "ERROR in BitmapSprite::load_frame() : Loaded sprite frame doesn't have the same size as the sprite itself." << std::endl;
      return false;
    }
    return true;
  }
  
  void save_frame(int anim_frame, const std::string& file_path)
  {
    auto* texture = fetch_frame(anim_frame);
    texture->save(file_path);
  }
  
  virtual void clone_frame(int anim_frame, int from_anim_frame) override
  {
    const auto N = stlutils::sizeI(texture_frames);
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
  
  void set_frame(int anim_frame, const drawing::Texture& texture)
  {
    auto* texture_dst = fetch_frame(anim_frame);
    *texture_dst = texture;
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
  bool set_sprite_chars_from_strings(int anim_frame, Strings... rows)
  {
    auto* texture = fetch_frame(anim_frame);
    
    std::array<std::string, sizeof...(rows)> row_array = { rows... };
    
    // Check that the number of rows matches the texture's height
    if (stlutils::sizeI(row_array) != texture->size.r)
    {
      std::cerr << "ERROR in BitmapSprite::set_sprite_chars_from_strings() : Number of strings must match the number of rows." << std::endl;
      return false;
    }
    
    for (const auto& row : row_array)
      if (stlutils::sizeI(row) != texture->size.c)
      {
        std::cerr << "ERROR in BitmapSprite::set_sprite_chars_from_strings() : Each string must have exactly NC characters." << std::endl;
        return false;
      }
    
    // Unpack strings into the characters vector
    int idx = 0;
    for (const auto& row : row_array)
      for (char ch : row)
        texture->characters[idx++] = ch;
        
    return true;
  }
  
  // Set sprite foreground colors
  template<typename... Colors>
  bool set_sprite_fg_colors(int anim_frame, Colors... fg_color)
  {
    auto* texture = fetch_frame(anim_frame);
    return set_sprite_data(texture->fg_colors, fg_color...);
  }
  
  // Set sprite foreground colors
  template<typename... Colors>
  bool set_sprite_fg_colors(int anim_frame, const ttl::Rectangle& bb, Colors... fg_color)
  {
    auto* texture = fetch_frame(anim_frame);
    return set_sprite_data(texture->fg_colors, bb, fg_color...);
  }
  
  // Set sprite background colors
  template<typename... Colors>
  bool set_sprite_bg_colors(int anim_frame, Colors... bg_color)
  {
    auto* texture = fetch_frame(anim_frame);
    return set_sprite_data(texture->bg_colors, bg_color...);
  }
  
  // Set sprite background colors
  template<typename... Colors>
  bool set_sprite_bg_colors(int anim_frame, const ttl::Rectangle& bb, Colors... bg_color)
  {
    auto* texture = fetch_frame(anim_frame);
    return set_sprite_data(texture->bg_colors, bb, bg_color...);
  }
  
  // Set sprite materials
  template<typename... Materials>
  bool set_sprite_materials(int anim_frame, Materials... mat)
  {
    auto* texture = fetch_frame(anim_frame);
    return set_sprite_data(texture->materials, mat...);
  }
  
  // Set sprite materials
  template<typename... Materials>
  bool set_sprite_materials(int anim_frame, const ttl::Rectangle& bb, Materials... mat)
  {
    auto* texture = fetch_frame(anim_frame);
    return set_sprite_data(texture->materials, bb, mat...);
  }
  
  const drawing::Texture& get_curr_frame(int sim_frame) const
  {
    int frame_id = func_calc_anim_frame(sim_frame);
    if (frame_id >= stlutils::sizeI(texture_frames))
    {
      std::cerr << "ERROR in BitmapSprite::get_curr_frame() : Incorrect frame id: " + std::to_string(frame_id) + " for sprite \"" + name + "\"! Sprite only has " + std::to_string(texture_frames.size()) + " frames." << std::endl;
      return;
    }
    return *texture_frames[frame_id];
  }
  
  virtual int num_frames() const override
  {
    return stlutils::sizeI(texture_frames);
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
  
  virtual AABB<int> calc_curr_AABB(int /*sim_frame*/) const override
  {
    return { pos.r, pos.c, size.r, size.c };
  }
  
  virtual Vec2 calc_curr_centroid(int /*sim_frame*/) const override
  {
    return { static_cast<float>(pos.r) + size.r * 0.5f, static_cast<float>(pos.c) + size.c * 0.5f };
  }
  
  virtual bool_vector calc_curr_coll_mask(int sim_frame, int collision_material) override
  {
    const auto& texture = get_curr_frame(sim_frame);
    const auto num_mats = stlutils::sizeI(texture.materials);
    bool_vector coll_mask(num_mats);
    for (int mat_idx = 0; mat_idx < num_mats; ++mat_idx)
      coll_mask[mat_idx] = (texture.materials[mat_idx] == collision_material);
    return coll_mask;
  }
  
  virtual Vec2 get_pos_offs(int /*sim_frame*/) const override
  {
    return {};
  }
};

// /////////////////////////////////////////////////

class VectorSprite : public Sprite
{
  struct LineSeg
  {
    std::array<Vec2, 2> pos;
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
    while (stlutils::sizeI(vector_frames) <= anim_frame)
      vector_frames.emplace_back(std::make_unique<VectorFrame>());
    return vector_frames[anim_frame].get();
  }
  
  std::pair<Vec2, Vec2> calc_seg_world_pos_flt(const LineSeg& line_seg) const
  {
    const auto aspect_ratio = 1.5f;
    auto rr0 = line_seg.pos[0].r;
    auto cc0 = line_seg.pos[0].c;
    auto rr1 = line_seg.pos[1].r;
    auto cc1 = line_seg.pos[1].c;
    float C = std::cos(rot_rad);
    float S = std::sin(rot_rad);
    auto r0 = pos.r + (C*rr0 - S*cc0);
    auto c0 = pos.c + (S*rr0 + C*cc0)*aspect_ratio;
    auto r1 = pos.r + (C*rr1 - S*cc1);
    auto c1 = pos.c + (S*rr1 + C*cc1)*aspect_ratio;
    Vec2 p0 { r0, c0 };
    Vec2 p1 { r1, c1 };
    return { p0, p1 };
  }
  
  std::pair<RC, RC> calc_seg_world_pos_round(const LineSeg& line_seg) const
  {
    auto [v0, v1] = calc_seg_world_pos_flt(line_seg);
    return { to_RC_round(v0), to_RC_round(v1) };
  }
  
public:
  VectorSprite(const std::string& a_name) : Sprite(a_name) {}
  
  void add_line_segment(int anim_frame, const Vec2& p0, const Vec2& p1, char ch, const styles::Style& style, int mat = 0)
  {
    auto* vector_frame = fetch_frame(anim_frame);
    auto& line_seg = vector_frame->line_segments.emplace_back();
    line_seg.pos[0] = p0;
    line_seg.pos[1] = p1;
    line_seg.ch = ch;
    line_seg.style = style;
    line_seg.mat = mat;
  }
  
  virtual void clone_frame(int anim_frame, int from_anim_frame) override
  {
    const auto N = stlutils::sizeI(vector_frames);
    if (from_anim_frame < N)
    {
      if (anim_frame >= N)
      {
        auto* frame_from = fetch_frame(from_anim_frame);
        fetch_frame(anim_frame);
        vector_frames[anim_frame] = std::make_unique<VectorFrame>(*frame_from);
      }
      else
        std::cout << "ERROR in clone_frame() : anim_frame must be larger than or equal to the number of vector frames!" << std::endl;
    }
    else
      std::cout << "ERROR in clone_frame() : from_anim_frame cannot be larger than or equal to the number of vector frames!" << std::endl;
  }
  
  void set_frame(int anim_frame, const VectorFrame& frame)
  {
    auto* frame_dst = fetch_frame(anim_frame);
    *frame_dst = frame;
  }
  
  const VectorFrame& get_curr_frame(int sim_frame) const
  {
    int frame_id = func_calc_anim_frame(sim_frame);
    if (frame_id >= stlutils::sizeI(vector_frames))
    {
      std::cerr << "ERROR in VectorSprite::get_curr_frame() : Incorrect frame id: " + std::to_string(frame_id) + " for sprite \"" + name + "\"! Sprite only has " + std::to_string(vector_frames.size()) + " frames." << std::endl;
      return;
    }
    return *vector_frames[frame_id];
  }
  
  virtual int num_frames() const override
  {
    return stlutils::sizeI(vector_frames);
  }
  
  void set_rotation(float rot_deg)
  {
    rot_rad = math::deg2rad(rot_deg);
  }
  
  float get_rotation() const
  {
    return math::rad2deg(rot_rad);
  }
  
  template<int NR, int NC>
  void draw(ScreenHandler<NR, NC>& sh, int sim_frame)
  {
    auto& vector_frame = get_curr_frame(sim_frame);
    
    for (const auto& line_seg : vector_frame.line_segments)
    {
      auto [p0, p1] = calc_seg_world_pos_round(line_seg);
      bresenham::plot_line(sh, p0, p1, std::string(1, line_seg.ch), line_seg.style.fg_color, line_seg.style.bg_color);
    }
  }
  
  virtual AABB<int> calc_curr_AABB(int sim_frame) const override
  {
    auto& vector_frame = get_curr_frame(sim_frame);

    AABB<int> aabb;
    for (const auto& line_seg : vector_frame.line_segments)
    {
      auto [p0, p1] = calc_seg_world_pos_round(line_seg);
      aabb.add_point(p0);
      aabb.add_point(p1);
    }
    return aabb;
  }
  
  virtual Vec2 calc_curr_centroid(int sim_frame) const override
  {
    auto& vector_frame = get_curr_frame(sim_frame);
  
    Vec2 centroid;
    int ctr = 0;
    for (const auto& line_seg : vector_frame.line_segments)
    {
      auto [p0, p1] = calc_seg_world_pos_flt(line_seg);
      centroid += p0;
      centroid += p1;
      ctr += 2;
    }
    centroid /= static_cast<float>(ctr);
    return centroid;
  }
  
  virtual bool_vector calc_curr_coll_mask(int sim_frame, int collision_material) override
  {
    auto aabb = calc_curr_AABB(sim_frame);
    auto rmin = aabb.r_min();
    auto cmin = aabb.c_min();
    const int num_points = aabb.width()*aabb.height();
    bool_vector coll_mask(num_points);
    std::vector<RC> points;
    const auto& vector_frame = get_curr_frame(sim_frame);
    for (const auto& line_seg : vector_frame.line_segments)
    {
      if (line_seg.mat != collision_material)
        continue;
      points.clear();
      auto [p0, p1] = calc_seg_world_pos_round(line_seg);
      bresenham::plot_line(p0, p1, points);
      for (const auto& pt : points)
        coll_mask[(pt.r - rmin) * aabb.width() + (pt.c - cmin)] = true;
    }
    return coll_mask;
  }
  
  virtual Vec2 get_pos_offs(int sim_frame) const override
  {
    auto aabb = calc_curr_AABB(sim_frame);
    Vec2 p0 { static_cast<float>(aabb.r_min()), static_cast<float>(aabb.c_min()) };
    return p0 - calc_curr_centroid(sim_frame);
  }
};

// /////////////////////////////////////////////////
// /////////////////////////////////////////////////

class SpriteHandler
{
  std::map<std::string, std::unique_ptr<Sprite>> m_sprites;
  
  void render(int sim_frame, std::function<void(Sprite*, int)> pred) const
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
          pred(sprite.get(), sim_frame);
      }
    }
  }

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
  
  Sprite* clone_sprite(const std::string& sprite_name, const std::string& from_sprite_name)
  {
    auto sprite_src = fetch_sprite(from_sprite_name);
    if (sprite_src == nullptr)
      return nullptr;
      
    auto f_copy_base = [](Sprite* dst, Sprite* src)
    {
      dst->pos = src->pos;
      dst->layer_id = src->layer_id;
      dst->enabled = src->enabled;
      dst->func_calc_anim_frame = src->func_calc_anim_frame;
    };
    
    if (auto* sprite_src_bitmap = dynamic_cast<BitmapSprite*>(sprite_src); sprite_src_bitmap != nullptr)
    {
      auto* sprite_dst_bitmap = create_bitmap_sprite(sprite_name);
      f_copy_base(sprite_dst_bitmap, sprite_src);
      auto size = sprite_src_bitmap->get_size();
      sprite_dst_bitmap->init(size.r, size.c);
      for (int frame_id = 0; frame_id < sprite_src_bitmap->num_frames(); ++frame_id)
      {
        auto& texture = sprite_src_bitmap->get_curr_frame(frame_id);
        sprite_dst_bitmap->set_frame(frame_id, texture);
      }
      return sprite_dst_bitmap;
    }
    if (auto* sprite_src_vector = dynamic_cast<VectorSprite*>(sprite_src); sprite_src_vector != nullptr)
    {
      auto* sprite_dst_vector = create_vector_sprite(sprite_name);
      f_copy_base(sprite_dst_vector, sprite_src);
      sprite_dst_vector->set_rotation(sprite_src_vector->get_rotation());
      for (int frame_id = 0; frame_id < sprite_src_vector->num_frames(); ++frame_id)
      {
        auto& frame = sprite_src_vector->get_curr_frame(frame_id);
        sprite_dst_vector->set_frame(frame_id, frame);
      }
      return sprite_dst_vector;
    }
    
    return nullptr;
  }
  
  template<int NS>
  std::array<Sprite*, NS> clone_sprite_array(const std::string& sprite_base_name, const std::string& from_sprite_name)
  {
    std::array<Sprite*, NS> sprite_arr;
    
    for (int sprite_idx = 0; sprite_idx < NS; ++sprite_idx)
      sprite_arr[sprite_idx] = clone_sprite(sprite_base_name + std::to_string(sprite_idx), from_sprite_name);
    
    return sprite_arr;
  }
  
  template<int NR, int NC>
  void draw(ScreenHandler<NR, NC>& sh, int sim_frame) const
  {
    render(sim_frame, [&sh](Sprite* sprite, int sim_frame)
    {
      if (auto* bitmap_sprite = dynamic_cast<BitmapSprite*>(sprite); bitmap_sprite != nullptr)
        bitmap_sprite->draw(sh, sim_frame);
      else if (auto* vector_sprite = dynamic_cast<VectorSprite*>(sprite); vector_sprite != nullptr)
        vector_sprite->draw(sh, sim_frame);
    });
  }
  
  template<int NR, int NC>
  void draw_dbg_bb(ScreenHandler<NR, NC>& sh, int sim_frame) const
  {
    render(sim_frame, [&sh](Sprite* sprite, int sim_frame)
    {
      auto aabb = sprite->calc_curr_AABB(sim_frame);

      auto rec = aabb.to_rectangle();
      drawing::draw_box_outline(sh, rec, drawing::OutlineType::Line, { Color::LightGray, Color::Transparent2 });
    });
  }
  
  template<int NR, int NC>
  void draw_dbg_pts(ScreenHandler<NR, NC>& sh, int sim_frame) const
  {
    render(sim_frame, [&sh](Sprite* sprite, int sim_frame)
    {
      auto pos = sprite->pos;
      sh.write_buffer("O", pos.r, pos.c, Color::DarkGray);
      
      auto centroid = to_RC_floor(sprite->calc_curr_centroid(sim_frame));
      sh.write_buffer("x", centroid.r, centroid.c, Color::DarkYellow);
    });
  }
};
