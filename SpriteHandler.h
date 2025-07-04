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
#include <Core/bool_vector.h>
#include <map>
#include <set>
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
  
  virtual bool_vector calc_curr_mask(int sim_frame, const std::vector<int>& mask_materials) = 0;
  
  virtual bool calc_cm() const = 0;
  
  virtual bool is_opaque(int sim_frame, const RC& pt) const = 0;
  
  virtual std::vector<RC> get_opaque_points(int sim_frame) const = 0;
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
    auto N_trg = stlutils::sizeI(target);
    auto N_src = stlutils::sizeI(source);

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
        if (trg_index < N_trg && src_index < N_src)
          target[trg_index] = source[src_index];
      }
    }
    
    return true;
  }
  
  template<typename T>
  void fill_sprite_data(std::vector<T>& target, const ttl::Rectangle& bb, T arg)
  {
    int nr = std::min(bb.r_len, size.r);
    int nc = std::min(bb.c_len, size.c);
    
    auto N_trg = stlutils::sizeI(target);
    
    for (int i = 0; i < nr; ++i)
    {
      for (int j = 0; j < nc; ++j)
      {
        // Map bounding box (i, j) to the target sprite data
        int trg_row = bb.r + i;
        int trg_col = bb.c + j;
        
        // Calculate linear index into the target vector and src_data
        int trg_index = trg_row * size.c + trg_col;
        
        // Assign data to the target vector
        if (trg_index < N_trg)
          target[trg_index] = arg;
      }
    }
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
  
  drawing::Texture* try_get_frame(int anim_frame)
  {
    if (anim_frame < stlutils::sizeI(texture_frames))
      return texture_frames[anim_frame].get();
    return nullptr;
  }
  
  void set_frame(int anim_frame, const drawing::Texture& texture)
  {
    auto* texture_dst = fetch_frame(anim_frame);
    *texture_dst = texture;
  }
  
  // #FIXME: Perhaps move these varyadic functions to Texture for more versatility.
  
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
  
  template<typename... Chars>
  void set_sprite_chars_vert(int anim_frame, int r0, int r1, int c, Chars... ch)
  {
    ttl::Rectangle bb { r0, c, r1 - r0 + 1, 1 };
    set_sprite_chars(anim_frame, bb, ch...);
  }
  
  template<typename... Chars>
  void set_sprite_chars_horiz(int anim_frame, int r, int c0, int c1, Chars... ch)
  {
    ttl::Rectangle bb { r, c0, 1, c1 - c0 + 1 };
    set_sprite_chars(anim_frame, bb, ch...);
  }
  
  void set_sprite_char(int anim_frame, int r, int c, char ch)
  {
    auto* texture = fetch_frame(anim_frame);
    if (texture != nullptr)
      texture->set_textel_char(r, c, ch);
  }
  
  void fill_sprite_chars(int anim_frame, char ch)
  {
    auto* texture = fetch_frame(anim_frame);
    texture->characters.assign(area, ch);
  }
  
  void fill_sprite_chars(int anim_frame, const ttl::Rectangle& bb, char ch)
  {
    auto* texture = fetch_frame(anim_frame);
    fill_sprite_data(texture->characters, bb, ch);
  }
  
  void fill_sprite_chars_vert(int anim_frame, int r0, int r1, int c, char ch)
  {
    auto* texture = fetch_frame(anim_frame);
    if (texture != nullptr)
      for (int r = r0; r <= r1; ++r)
        texture->set_textel_char(r, c, ch);
  }
  
  void fill_sprite_chars_horiz(int anim_frame, int r, int c0, int c1, char ch)
  {
    auto* texture = fetch_frame(anim_frame);
    if (texture != nullptr)
      for (int c = c0; c <= c1; ++c)
        texture->set_textel_char(r, c, ch);
  }
  
  // Set sprite characters from a string for each row.
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
    
    // Unpack strings into the characters vector.
    int idx = 0;
    for (const auto& row : row_array)
      for (char ch : row)
        texture->characters[idx++] = ch;
        
    return true;
  }
  
  template<typename... Colors>
  bool set_sprite_fg_colors(int anim_frame, Colors... fg_color)
  {
    auto* texture = fetch_frame(anim_frame);
    return set_sprite_data(texture->fg_colors, fg_color...);
  }
  
  template<typename... Colors>
  bool set_sprite_fg_colors(int anim_frame, const ttl::Rectangle& bb, Colors... fg_color)
  {
    auto* texture = fetch_frame(anim_frame);
    return set_sprite_data(texture->fg_colors, bb, fg_color...);
  }
  
  template<typename... Colors>
  void set_sprite_fg_colors_vert(int anim_frame, int r0, int r1, int c, Colors... fg_color)
  {
    ttl::Rectangle bb { r0, c, r1 - r0 + 1, 1 };
    set_sprite_fg_colors(anim_frame, bb, fg_color...);
  }
  
  template<typename... Colors>
  void set_sprite_fg_colors_horiz(int anim_frame, int r, int c0, int c1, Colors... fg_color)
  {
    ttl::Rectangle bb { r, c0, 1, c1 - c0 + 1 };
    set_sprite_fg_colors(anim_frame, bb, fg_color...);
  }
  
  void set_sprite_fg_color(int anim_frame, int r, int c, Color fg_color)
  {
    auto* texture = fetch_frame(anim_frame);
    if (texture != nullptr)
      texture->set_textel_fg_color(r, c, fg_color);
  }
  
  void fill_sprite_fg_colors(int anim_frame, Color fg_color)
  {
    auto* texture = fetch_frame(anim_frame);
    texture->fg_colors.assign(area, fg_color);
  }
  
  void fill_sprite_fg_colors(int anim_frame, const ttl::Rectangle& bb, Color fg_color)
  {
    auto* texture = fetch_frame(anim_frame);
    fill_sprite_data(texture->fg_colors, bb, fg_color);
  }
  
  void fill_sprite_fg_colors_vert(int anim_frame, int r0, int r1, int c, Color fg_color)
  {
    auto* texture = fetch_frame(anim_frame);
    if (texture != nullptr)
      for (int r = r0; r <= r1; ++r)
        texture->set_textel_fg_color(r, c, fg_color);
  }
  
  void fill_sprite_fg_colors_horiz(int anim_frame, int r, int c0, int c1, Color fg_color)
  {
    auto* texture = fetch_frame(anim_frame);
    if (texture != nullptr)
      for (int c = c0; c <= c1; ++c)
        texture->set_textel_fg_color(r, c, fg_color);
  }
  
  template<typename... Colors>
  bool set_sprite_bg_colors(int anim_frame, Colors... bg_color)
  {
    auto* texture = fetch_frame(anim_frame);
    return set_sprite_data(texture->bg_colors, bg_color...);
  }
  
  template<typename... Colors>
  bool set_sprite_bg_colors(int anim_frame, const ttl::Rectangle& bb, Colors... bg_color)
  {
    auto* texture = fetch_frame(anim_frame);
    return set_sprite_data(texture->bg_colors, bb, bg_color...);
  }
  
  template<typename... Colors>
  void set_sprite_bg_colors_vert(int anim_frame, int r0, int r1, int c, Colors... bg_color)
  {
    ttl::Rectangle bb { r0, c, r1 - r0 + 1, 1 };
    set_sprite_bg_colors(anim_frame, bb, bg_color...);
  }
  
  template<typename... Colors>
  void set_sprite_bg_colors_horiz(int anim_frame, int r, int c0, int c1, Colors... bg_color)
  {
    ttl::Rectangle bb { r, c0, 1, c1 - c0 + 1 };
    set_sprite_bg_colors(anim_frame, bb, bg_color...);
  }
  
  void set_sprite_bg_color(int anim_frame, int r, int c, Color bg_color)
  {
    auto* texture = fetch_frame(anim_frame);
    if (texture != nullptr)
      texture->set_textel_bg_color(r, c, bg_color);
  }
  
  void fill_sprite_bg_colors(int anim_frame, Color bg_color)
  {
    auto* texture = fetch_frame(anim_frame);
    texture->bg_colors.assign(area, bg_color);
  }
  
  void fill_sprite_bg_colors(int anim_frame, const ttl::Rectangle& bb, Color bg_color)
  {
    auto* texture = fetch_frame(anim_frame);
    fill_sprite_data(texture->bg_colors, bb, bg_color);
  }
  
  void fill_sprite_bg_colors_vert(int anim_frame, int r0, int r1, int c, Color bg_color)
  {
    auto* texture = fetch_frame(anim_frame);
    if (texture != nullptr)
      for (int r = r0; r <= r1; ++r)
        texture->set_textel_bg_color(r, c, bg_color);
  }
  
  void fill_sprite_bg_colors_horiz(int anim_frame, int r, int c0, int c1, Color bg_color)
  {
    auto* texture = fetch_frame(anim_frame);
    if (texture != nullptr)
      for (int c = c0; c <= c1; ++c)
        texture->set_textel_bg_color(r, c, bg_color);
  }
  
  template<typename... Materials>
  bool set_sprite_materials(int anim_frame, Materials... mat)
  {
    auto* texture = fetch_frame(anim_frame);
    return set_sprite_data(texture->materials, mat...);
  }
  
  template<typename... Materials>
  bool set_sprite_materials(int anim_frame, const ttl::Rectangle& bb, Materials... mat)
  {
    auto* texture = fetch_frame(anim_frame);
    return set_sprite_data(texture->materials, bb, mat...);
  }
  
  template<typename... Materials>
  void set_sprite_materials_vert(int anim_frame, int r0, int r1, int c, Materials... mat)
  {
    ttl::Rectangle bb { r0, c, r1 - r0 + 1, 1 };
    set_sprite_materials(anim_frame, bb, mat...);
  }
  
  template<typename... Materials>
  void set_sprite_materials_horiz(int anim_frame, int r, int c0, int c1, Materials... mat)
  {
    ttl::Rectangle bb { r, c0, 1, c1 - c0 + 1 };
    set_sprite_materials(anim_frame, bb, mat...);
  }
  
  void set_sprite_material(int anim_frame, int r, int c, int mat)
  {
    auto* texture = fetch_frame(anim_frame);
    if (texture != nullptr)
      texture->set_textel_material(r, c, mat);
  }
  
  void fill_sprite_materials(int anim_frame, int mat)
  {
    auto* texture = fetch_frame(anim_frame);
    texture->materials.assign(area, mat);
  }
  
  void fill_sprite_materials(int anim_frame, const ttl::Rectangle& bb, int mat)
  {
    auto* texture = fetch_frame(anim_frame);
    fill_sprite_data(texture->materials, bb, mat);
  }
  
  void fill_sprite_materials_vert(int anim_frame, int r0, int r1, int c, int mat)
  {
    auto* texture = fetch_frame(anim_frame);
    if (texture != nullptr)
      for (int r = r0; r <= r1; ++r)
        texture->set_textel_material(r, c, mat);
  }
  
  void fill_sprite_materials_horiz(int anim_frame, int r, int c0, int c1, int mat)
  {
    auto* texture = fetch_frame(anim_frame);
    if (texture != nullptr)
      for (int c = c0; c <= c1; ++c)
        texture->set_textel_material(r, c, mat);
  }
  
  // Replace args mean they replace the attribute if the main argument/attribute was found on that pixel.
  // E.g. if bg_color was found on a pixel, then bg_color_replace will be used instead, if set.
  // If a replace argument is set but not the main argument, then nothing will happen with that attribute.
  // E.g. if ch is nullopt but ch_replace is non-nullopt, then it is the same as if both are nullopt.
  // This mechanism allows you to do penumbra / umbra shading techniques and stuff like that.
  bool plot_line(int sim_frame, const RC& p0, const RC& p1,
                 std::optional<char> ch, std::optional<char> ch_replace,
                 std::optional<Color> fg_color, std::optional<Color> fg_color_replace,
                 std::optional<Color> bg_color, std::optional<Color> bg_color_replace,
                 std::optional<int> mat, std::optional<int> mat_replace)
  {
    auto* texture = get_curr_sim_frame(sim_frame);
    if (texture == nullptr)
      return false;
    std::vector<RC> points;
    bresenham::plot_line(p0, p1, points);
    auto f_set_attribute = [](auto& dst, const auto& src, const auto& src_replace)
    {
      if (src.has_value() && src_replace.has_value() && dst == src.value())
        dst = src_replace.value();
      else
        dst = src.value_or(dst);
    };
    for (const auto& pt : points)
    {
      auto r = math::roundI(pt.r) - pos.r;
      auto c = math::roundI(pt.c) - pos.c;
      auto textel = (*texture)(r, c);
      f_set_attribute(textel.ch, ch, ch_replace);
      f_set_attribute(textel.fg_color, fg_color, fg_color_replace);
      f_set_attribute(textel.bg_color, bg_color, bg_color_replace);
      f_set_attribute(textel.mat, mat, mat_replace);
      texture->set_textel(r, c, textel);
    }
    return true;
  }
  
  void flip_ud(int anim_frame)
  {
    auto f_flip_char = [](drawing::Textel& txt)
    {
      auto& ch = txt.ch;
      switch(ch)
      {
        case '_': ch = '-'; break;
        case 'W': ch = 'M'; break;
        case 'M': ch = 'W'; break;
        case 'w': ch = 'm'; break;
        case 'm': ch = 'w'; break;
        case '^': ch = 'v'; break;
        case 'v': ch = '^'; break;
        case '/': ch = '\\'; break;
        case '\\': ch = '/'; break;
        case 'b': ch = 'p'; break;
        case 'p': ch = 'b'; break;
        case 'z': ch = 's'; break;
        case 's': ch = 'z'; break;
        case 'Z': ch = 'S'; break;
        case 'S': ch = 'Z'; break;
        default: break;
      }
    };
  
    auto* texture = fetch_frame(anim_frame);
    const int half_height = size.r/2;
    for (int c = 0; c < size.c; ++c)
    {
      for (int r = 0; r < half_height; ++r)
      {
        int r_inv = size.r - r - 1;
        auto a = texture->operator()(r, c);
        auto b = texture->operator()(r_inv, c);
        f_flip_char(a);
        f_flip_char(b);
        texture->set_textel(r, c, b);
        texture->set_textel(r_inv, c, a);
      }
    }
  }
  
  void flip_ud()
  {
    auto num_frames = stlutils::sizeI(texture_frames);
    for (int anim_frame = 0; anim_frame < num_frames; ++anim_frame)
      flip_ud(anim_frame);
  }
  
  void flip_lr(int anim_frame)
  {
    auto f_flip_char = [](drawing::Textel& txt)
    {
      auto& ch = txt.ch;
      switch(ch)
      {
        case '/': ch = '\\'; break;
        case '\\': ch = '/'; break;
        case '(': ch = ')'; break;
        case ')': ch = '('; break;
        case '[': ch = ']'; break;
        case ']': ch = '['; break;
        case '}': ch = '{'; break;
        case 'd': ch = 'b'; break;
        case 'b': ch = 'd'; break;
        case 'J': ch = 'L'; break;
        case 'L': ch = 'J'; break;
        case '<': ch = '>'; break;
        case '>': ch = '<'; break;
        case 'z': ch = 's'; break;
        case 's': ch = 'z'; break;
        case 'Z': ch = 'S'; break;
        case 'S': ch = 'Z'; break;
        default: break;
      }
    };
  
    auto* texture = fetch_frame(anim_frame);
    const int half_width = size.c/2;
    for (int r = 0; r < size.r; ++r)
    {
      for (int c = 0; c < half_width; ++c)
      {
        int c_inv = size.c - c - 1;
        auto a = texture->operator()(r, c);
        auto b = texture->operator()(r, c_inv);
        f_flip_char(a);
        f_flip_char(b);
        texture->set_textel(r, c, b);
        texture->set_textel(r, c_inv, a);
      }
    }
  }
  
  void flip_lr()
  {
    auto num_frames = stlutils::sizeI(texture_frames);
    for (int anim_frame = 0; anim_frame < num_frames; ++anim_frame)
      flip_lr(anim_frame);
  }
  
  drawing::Texture* get_curr_sim_frame(int sim_frame) const
  {
    int frame_id = func_calc_anim_frame(sim_frame);
    return get_curr_local_frame(frame_id);
  }
  
  drawing::Texture* get_curr_local_frame(int frame_id) const
  {
    if (frame_id >= stlutils::sizeI(texture_frames))
    {
      std::cerr << "ERROR in BitmapSprite::get_curr_frame() : Incorrect frame id: " + std::to_string(frame_id) + " for sprite \"" + name + "\"! Sprite only has " + std::to_string(texture_frames.size()) + " frames." << std::endl;
      return nullptr;
    }
    return texture_frames[frame_id].get();
  }
  
  virtual int num_frames() const override
  {
    return stlutils::sizeI(texture_frames);
  }
  
  template<int NR, int NC>
  bool draw(ScreenHandler<NR, NC>& sh, int sim_frame)
  {
    auto* texture = get_curr_sim_frame(sim_frame);
    if (texture == nullptr)
      return false;
    
    drawing::draw_box_textured(sh,
                               pos.r - 1, pos.c - 1,
                               texture->size.r + 2, texture->size.c + 2,
                               drawing::SolarDirection::Zenith,
                               *texture);
    
    return true;
  }
  
  virtual AABB<int> calc_curr_AABB(int /*sim_frame*/) const override
  {
    return { pos.r, pos.c, size.r, size.c };
  }
  
  virtual Vec2 calc_curr_centroid(int /*sim_frame*/) const override
  {
    return { static_cast<float>(pos.r) + size.r * 0.5f, static_cast<float>(pos.c) + size.c * 0.5f };
  }
  
  virtual bool_vector calc_curr_mask(int sim_frame, const std::vector<int>& mask_materials) override
  {
    const auto* texture = get_curr_sim_frame(sim_frame);
    if (texture == nullptr)
      return {};
    const auto num_mats = stlutils::sizeI(texture->materials);
    bool_vector mask(num_mats);
    for (int mat_idx = 0; mat_idx < num_mats; ++mat_idx)
      mask[mat_idx] = stlutils::contains(mask_materials, texture->materials[mat_idx]);
    return mask;
  }
  
  virtual bool calc_cm() const override { return true; }
  
  virtual bool is_opaque(int sim_frame, const RC& pt) const override
  {
    const auto* texture = get_curr_sim_frame(sim_frame);
    if (texture == nullptr)
      return false;
      
    int r = pt.r - pos.r;
    int c = pt.c - pos.c;
    auto textel = (*texture)(r, c);
    return !(textel.bg_color == Color::Transparent || textel.bg_color == Color::Transparent2);
  }
  
  virtual std::vector<RC> get_opaque_points(int sim_frame) const override
  {
    const auto* texture = get_curr_sim_frame(sim_frame);
    if (texture == nullptr)
      return {};
      
    auto aabb = calc_curr_AABB(sim_frame);
    std::vector<RC> opaque_points;
    for (int r = aabb.r_min(); r <= aabb.r_max(); ++r)
    {
      for (int c = aabb.c_min(); c <= aabb.c_max(); ++c)
      {
        auto textel = (*texture)(r - pos.r, c - pos.c);
        if (!(textel.bg_color == Color::Transparent || textel.bg_color == Color::Transparent2))
          opaque_points.emplace_back(RC {r, c});
      }
    }

    return opaque_points;
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
    bool fill_closed_polylines = false; // not yet implemented.
    char fill_char = 'S'; // not yet implemented.
    std::vector<std::vector<LineSeg>> closed_polylines;
    std::vector<LineSeg> open_polylines;
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
  
  //               p0
  //    +-----------+
  //    |        *  |
  //    |       ##  |
  //    |     ## #  |
  //    |   ##   #  |
  // p2 | *# o x  # | o = (0, 0) local origo from line segments' coordsys. Corresponds to Sprite::pos in world coords.
  //    |   ###   # | x = centre of mass. We rotate points around this point.
  //    |      ###* |
  //    +-----------+ p1
  // p0 : { -4, 4 } // local coordsys
  // p1 : { 2, 5 } // local coordsys
  // p2 : { 0, -3 } // local coordsys
  
  // 1.
  // (5, 5) AABB pos (world coordsys) from Sprite::pos + linesegs.
  // +-----------+
  // |           |
  // |       ##  |
  // |     ## #  |
  // |   ##   #  |
  // | ## o    # | o = (0, 0) @ Sprite::pos = { 3, 5 } (world coordsys).
  // |   ###   # |
  // |      #### |
  // +-----------+
  
  // 2.
  //
  // +-----------+
  // |           |
  // |       ##  |
  // |     ## #  |
  // |   ##   #  |
  // | ## o x  # | o = (0, 0) @ Sprite::pos = { 3, 5 } (world coordsys).
  // |   ###   # | x = centre of mass = ~{ 3, 7 } (world coordsys).
  // |      #### |
  // +-----------+
  // x = { 0.55, 2 } (local coordsys), { 3.55, 7 } (world coordsys) (Bresenham)
  // x = { -0.6667   2.0000 } (local coordsys), { 2.3333, 7 } (world coordsys) (Points)
  // x = { 0.2727   2.0303 } (local coordsys), { 3, 7 } (world coordsys) (Bresenham, filled) <-- most correct. aspect ratio's fault?
  
  // 3. Rotate around x.
  
  // 4. Deduce new o from x.
  
  // New idea: The user designs the sprite around an assumed center of mass.
  // So "o" becomes the center of mass instead.
  
  
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
    
  void finalize_topology(int anim_frame)
  {
    struct LineSegData
    {
      int idx = -1;
      int v_idx = -1;
    };
    std::vector<Vec2> positions;
    std::map<int, std::vector<LineSegData>> pos_lineseg_map;
    float c_snap_dist_sq = 1e-6f;
    
    auto* vector_frame = fetch_frame(anim_frame);
    //vector_frame->open_polylines.emplace_back(line_seg);
    vector_frame->open_polylines = vector_frame->line_segments;
    auto N = stlutils::sizeI(vector_frame->open_polylines);
    
    //     p1   s0:01   p5
    //         +------+
    //  s3:10 /        \ s1:01
    //    p0 +          \
    //        \          + p3
    //   s2:10 \        / s5:10
    //       p4 +------+ p2
    //           s4:01
    //
    // p0 -> s3:1*, s2:1
    // p1 -> s3:0*, s0:0*
    // p2 -> s4:1, s5:1
    // p3 -> s5:0, s1:1*
    // p4 -> s2:0, s4:0
    // p5 -> s0:1*, s1:0*
    // p0 -> s3:1 -> s3:0 -> p1 -> s0:0 -> s0:1 -> p5 -> s1:0 -> s1:1 -> p3 ->
    
    // 1. Register all positions.
    for (const auto& ls : vector_frame->open_polylines)
    {
      const auto& p0 = ls.pos[0];
      const auto& p1 = ls.pos[1];
      stlutils::emplace_back_if_not(positions, p0, [c_snap_dist_sq, &p0](const Vec2& pos)
        { return math::distance_squared(pos, p0) < c_snap_dist_sq; });
      stlutils::emplace_back_if_not(positions, p1, [c_snap_dist_sq, &p1](const Vec2& pos)
        { return math::distance_squared(pos, p1) < c_snap_dist_sq; });
    }
    // 2. Map position to lineseg and its vtx-idx.
    for (int ls_idx = 0; ls_idx < N; ++ls_idx)
    {
      const auto& ls = vector_frame->open_polylines[ls_idx];
      const auto& p0 = ls.pos[0];
      const auto& p1 = ls.pos[1];
      auto idx_pos_lsv0 = stlutils::find_if_idx(positions, [c_snap_dist_sq, &p0](const auto& pos)
        { return math::distance_squared(pos, p0) < c_snap_dist_sq; });
      auto idx_pos_lsv1 = stlutils::find_if_idx(positions, [c_snap_dist_sq, &p1](const auto& pos)
        { return math::distance_squared(pos, p1) < c_snap_dist_sq; });
      if (idx_pos_lsv0 != -1)
      {
        LineSegData lsd { ls_idx, 0 };
        pos_lineseg_map[idx_pos_lsv0].emplace_back(lsd);
      }
      if (idx_pos_lsv1 != -1)
      {
        LineSegData lsd { ls_idx, 1 };
        pos_lineseg_map[idx_pos_lsv1].emplace_back(lsd);
      }
    }
    
    // 3. Try to connect linesegs via positions.
    
    bool_vector visited(N, false); // Mark line segments.
    std::vector<int> path_indices;
    std::set<int> visited_positions;
    
    for (int start_idx = 0; start_idx < N; ++start_idx)
    {
      if (visited[start_idx])
        continue;
        
      path_indices.clear();
      visited_positions.clear();
      
      int curr_idx = start_idx;
      int curr_vtx = 0; // Can also try both ends.
      
      for (;;)
      {
        const auto& seg = vector_frame->open_polylines[curr_idx];
        visited[curr_idx] = true;
        path_indices.emplace_back(curr_idx);
        
        Vec2 next_pos = seg.pos[1 - curr_vtx];
        int next_pos_idx = stlutils::find_if_idx(positions,
                                                 [c_snap_dist_sq, &next_pos](const Vec2& pos)
                                                 { return math::distance_squared(pos, next_pos) < c_snap_dist_sq; });
        
        if (next_pos_idx == -1 || visited_positions.count(next_pos_idx) > 0)
          break; // Either disconnected or looping over visited node.
        
        visited_positions.insert(next_pos_idx);
        
        // Closed loop check.
        int start_pos_idx = stlutils::find_if_idx(positions,
                                                  [c_snap_dist_sq, &vector_frame, start_idx, curr_vtx](const Vec2& pos)
        {
          return math::distance_squared(pos, vector_frame->open_polylines[start_idx].pos[curr_vtx]) < c_snap_dist_sq;
        });
        if (next_pos_idx == start_pos_idx)
        {
          // Closed loop detected!
          std::vector<LineSeg> closed;
          for (int idx : path_indices)
            closed.emplace_back(vector_frame->open_polylines[idx]);
          
          vector_frame->closed_polylines.emplace_back(std::move(closed));
          
          // Remove from open_polylines later
          for (int idx : path_indices)
            visited[idx] = true;
          
          break;
        }
        
        const auto& candidates = pos_lineseg_map[next_pos_idx];
        
        // Try to find unvisited segment.
        bool found = false;
        for (const auto& lsd : candidates)
        {
          if (!visited[lsd.idx])
          {
            curr_idx = lsd.idx;
            curr_vtx = lsd.v_idx;
            found = true;
            break;
          }
        }
        
        if (!found)
          break;
      }
    }
    
    // Remove visited from open_polylines (in reverse to not break indices).
    for (int i = N - 1; i >= 0; --i)
      if (visited[i])
        stlutils::erase_at(vector_frame->open_polylines, i);
  }
  
  void add_line_segment(int anim_frame, const Vec2& p0, const Vec2& p1, const styles::Style& style, int mat = 0)
  {
    add_line_segment(anim_frame, p0, p1, -1, style, mat);
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
  
  VectorFrame* get_curr_sim_frame(int sim_frame) const
  {
    int frame_id = func_calc_anim_frame(sim_frame);
    return get_curr_local_frame(frame_id);
  }
  
  VectorFrame* get_curr_local_frame(int frame_id) const
  {
    if (frame_id >= stlutils::sizeI(vector_frames))
    {
      std::cerr << "ERROR in VectorSprite::get_curr_frame() : Incorrect frame id: " + std::to_string(frame_id) + " for sprite \"" + name + "\"! Sprite only has " + std::to_string(vector_frames.size()) + " frames." << std::endl;
      return nullptr;
    }
    return vector_frames[frame_id].get();
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
  bool draw(ScreenHandler<NR, NC>& sh, int sim_frame)
  {
    auto* vector_frame = get_curr_sim_frame(sim_frame);
    if (vector_frame == nullptr)
      return false;
    
    for (const auto& line_seg : vector_frame->line_segments)
    {
      auto [p0, p1] = calc_seg_world_pos_round(line_seg);
      
      char ch = 0;
      if (line_seg.ch == -1)
      {
        auto dir = p0 - p1;
        if (!(dir.r == 0 && dir.c == 0))
        {
          auto dr = static_cast<float>(dir.r);
          auto dc = static_cast<float>(dir.c);
          auto lineseg_rot_deg = math::rad2deg(std::atan2(dr, dc));
          if (math::in_range<float>(lineseg_rot_deg, -180.f, -158.f, Range::ClosedOpen))
            ch = '-';
          else if (math::in_range<float>(lineseg_rot_deg, -158.f, -112.f, Range::ClosedOpen))
            ch = '\\';
          else if (math::in_range<float>(lineseg_rot_deg, -112.f, -68.f, Range::ClosedOpen))
            ch = '|';
          else if (math::in_range<float>(lineseg_rot_deg, -68.f, -22.f, Range::ClosedOpen))
            ch = '/';
          else if (math::in_range<float>(lineseg_rot_deg, -22.f, 22.f, Range::ClosedOpen))
            ch = '-';
          else if (math::in_range<float>(lineseg_rot_deg, 22.f, 68.f, Range::ClosedOpen))
            ch = '\\';
          else if (math::in_range<float>(lineseg_rot_deg, 68.f, 112.f, Range::ClosedOpen))
            ch = '|';
          else if (math::in_range<float>(lineseg_rot_deg, 112.f, 158.f, Range::ClosedOpen))
            ch = '/';
          else if (math::in_range<float>(lineseg_rot_deg, 158.f, 180.f, Range::Closed))
            ch = '-';
          else
            throw std::invalid_argument(std::to_string(lineseg_rot_deg));
        }
      }
      else
        ch = line_seg.ch;
      
      bresenham::plot_line(sh, p0, p1, std::string(1, ch), line_seg.style.fg_color, line_seg.style.bg_color);
    }
    
    return true;
  }
  
  virtual AABB<int> calc_curr_AABB(int sim_frame) const override
  {
    auto* vector_frame = get_curr_sim_frame(sim_frame);
    if (vector_frame == nullptr)
      return {};

    AABB<int> aabb;
    for (const auto& line_seg : vector_frame->line_segments)
    {
      auto [p0, p1] = calc_seg_world_pos_round(line_seg);
      aabb.add_point(p0);
      aabb.add_point(p1);
    }
    return aabb;
  }
  
  virtual Vec2 calc_curr_centroid(int sim_frame) const override
  {
    return to_Vec2(pos); // Assuming you designed the sprite around the centroid / CoM.
  }
  
  virtual bool_vector calc_curr_mask(int sim_frame, const std::vector<int>& mask_materials) override
  {
    const auto* vector_frame = get_curr_sim_frame(sim_frame);
    if (vector_frame == nullptr)
      return {};
  
    auto aabb = calc_curr_AABB(sim_frame);
    auto rmin = aabb.r_min();
    auto cmin = aabb.c_min();
    const int num_points = aabb.width()*aabb.height();
    bool_vector mask(num_points);
    std::vector<RC> points;
    for (const auto& line_seg : vector_frame->line_segments)
    {
      if (!stlutils::contains(mask_materials, line_seg.mat))
        continue;
      points.clear();
      auto [p0, p1] = calc_seg_world_pos_round(line_seg);
      bresenham::plot_line(p0, p1, points);
      for (const auto& pt : points)
        mask[(pt.r - rmin) * aabb.width() + (pt.c - cmin)] = true;
    }
    return mask;
  }
  
  virtual bool calc_cm() const override { return false; }
  
  virtual bool is_opaque(int sim_frame, const RC& pos) const override
  {
    const auto* vector_frame = get_curr_sim_frame(sim_frame);
    if (vector_frame == nullptr)
      return false;
      
    auto aabb = calc_curr_AABB(sim_frame);
    if (!aabb.contains(pos))
      return false;

    std::vector<RC> points;
    for (const auto& line_seg : vector_frame->line_segments)
    {
      points.clear();
      auto [p0, p1] = calc_seg_world_pos_round(line_seg);
      bresenham::plot_line(p0, p1, points);
      for (const auto& pt : points)
        if (pt == pos)
          return true;
    }
    return false;
  }
  
  virtual std::vector<RC> get_opaque_points(int sim_frame) const override
  {
    const auto* vector_frame = get_curr_sim_frame(sim_frame);
    if (vector_frame == nullptr)
      return {};
    
    std::vector<RC> opaque_points;
    for (const auto& line_seg : vector_frame->line_segments)
    {
      auto [p0, p1] = calc_seg_world_pos_round(line_seg);
      bresenham::plot_line(p0, p1, opaque_points);
    }

    return opaque_points;
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
        auto* texture = sprite_src_bitmap->get_curr_local_frame(frame_id);
        if (texture != nullptr)
          sprite_dst_bitmap->set_frame(frame_id, *texture);
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
        auto* frame = sprite_src_vector->get_curr_local_frame(frame_id);
        if (frame != nullptr)
          sprite_dst_vector->set_frame(frame_id, *frame);
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
  
  void clear()
  {
    m_sprites.clear();
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
