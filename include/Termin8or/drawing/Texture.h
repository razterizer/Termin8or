//
//  Texture.h
//  Termin8or
//
//  Created by Rasmus Anthin on 2024-06-25.
//

#pragma once
#include "../geom/RC.h"
#include "../geom/Rectangle.h"
#include "../screen/Color.h"
#include "../screen/Styles.h"
#include "../screen/Ansi.h"
#include <Core/TextIO.h>
#include <Core/FolderHelper.h>
#include <Core/StringHelper.h>
#include <fstream>
#include <sstream>


namespace t8
{
  
  namespace texture
  {
    static constexpr uint8_t raw_mat_none = 255;
    static constexpr int mat_none = -1;
    
    inline bool has_raw_material(uint8_t m_raw) { return m_raw != raw_mat_none; }
    inline bool has_material(int m) { return m != mat_none; }
    
    // Encode to raw mat.
    inline uint8_t encode_raw_material(int m)
    {
      if (m < 0) return raw_mat_none;
      if (m > 254) return 254; // Or maybe assert instead?
      return static_cast<uint8_t>(m);
    }
    
    // Decode from raw mat.
    inline int decode_raw_material(uint8_t m_raw)
    {
      return (m_raw == raw_mat_none) ? mat_none : static_cast<int>(m_raw);
    }
    
    std::string raw_mat_to_str(uint8_t mat)
    {
      constexpr uint8_t span09 = 1 + (9 - 0);     // 10, 0-9
      constexpr uint8_t spanAZ = 1 + ('Z' - 'A'); // 26, A-Z, a-z
      constexpr uint8_t page_span = span09 + 2*spanAZ; // 62
      if (mat >= raw_mat_none)
        return "-";
        
      static constexpr std::array<std::string_view, 5> prefix_list { "", ".", ",", ":", ";" };
      int page = mat / page_span;
      int sub = mat % page_span;
      if (page >= static_cast<int>(prefix_list.size()))
        return "-";
      
      char ch_sub;
      if (sub < span09)
        ch_sub = static_cast<char>('0' + sub);
      else if (sub < span09 + spanAZ)
        ch_sub = static_cast<char>('A' + (sub - span09));
      else
        ch_sub = static_cast<char>('a' + (sub - span09 - spanAZ));
      
      std::string str_ret(prefix_list[page]);
      str_ret.push_back(ch_sub);
      return str_ret;
    }
    
    uint8_t str_to_raw_mat(std::string_view sv, int& pos)
    {
      constexpr uint8_t span09 = 1 + (9 - 0);     // 10, 0-9
      constexpr uint8_t spanAZ = 1 + ('Z' - 'A'); // 26, A-Z, a-z
      constexpr uint8_t page_span = span09 + 2*spanAZ; // 62
      if (sv.length() <= static_cast<size_t>(pos))
        return raw_mat_none;
        
      //char ch_prefix = 0;
      char ch = sv[pos];
      
      if (ch == '-')
      {
        pos++;
        return texture::raw_mat_none;
      }
      
      static constexpr std::array<char, 4> prefix_list { '.', ',', ':', ';' };
      
      auto idx_prefix = stlutils::find_idx(prefix_list, ch);
      int dp = 1;
      int mat_page_offs = 0;
      if (0 <= idx_prefix)
      {
        if (sv.length() <= static_cast<size_t>(pos + 1))
          return raw_mat_none;
        //ch_prefix = sv[pos];
        ch = sv[pos + 1];
        dp = 2;
        mat_page_offs = (idx_prefix + 1) * page_span;
      }
      
      auto f_check_range = [ch, &pos, dp](char ch_start, char ch_end, int offset, uint8_t& material) -> bool
      {
        if (ch_start <= ch && ch <= ch_end)
        {
          pos += dp;
          int mat = (ch - ch_start) + offset;
          if (mat >= raw_mat_none)
            material = raw_mat_none;
          else
            material = static_cast<uint8_t>(mat);
          return true;
        }
        return false;
      };
      
      uint8_t mat = raw_mat_none;
      if (f_check_range('0', '9', mat_page_offs, mat))
        return mat;
      if (f_check_range('A', 'Z', mat_page_offs + span09, mat))
        return mat;
      if (f_check_range('a', 'z', mat_page_offs + span09 + spanAZ, mat))
        return mat;
      
      return raw_mat_none;
    }
  }

  // size.r = 1, size.c = 1 yields a 1x1 texture.
  struct Textel
  {
    Glyph glyph = ' ';
    Color fg_color = Color16::Default;
    Color bg_color = Color16::Transparent2;
    uint8_t mat_raw = texture::raw_mat_none;
    
    Style get_style() const { return { fg_color, bg_color }; }
    void set_style(const Style& style)
    {
      fg_color = style.fg_color;
      bg_color = style.bg_color;
    }
    
    std::string raw_mat_to_str() const { return texture::raw_mat_to_str(mat_raw); }
    
    int decode_raw_mat() const { return texture::decode_raw_material(mat_raw); }
    void encode_raw_mat(int mat) { mat_raw = texture::encode_raw_material(mat); }
    
    bool operator==(const Textel& other) const
    {
      return this->glyph == other.glyph
        && this->fg_color == other.fg_color
        && this->bg_color == other.bg_color
        && this->mat_raw == other.mat_raw;
    }
  };
  
  struct Texture
  {
    static const int compatible_version_until_and_including = 30;
    int ver = 10;
    RC size;
    int area = 0;
    std::vector<Glyph> glyphs;
    std::vector<Color> fg_colors;
    std::vector<Color> bg_colors;
    std::vector<uint8_t> materials_raw;
    
    Texture() = default;
    Texture(const RC& tex_size)
      : size(tex_size)
      , area(tex_size.r*tex_size.c)
      , glyphs(area, ' ')
      , fg_colors(area, Color16::Default)
      , bg_colors(area, Color16::Transparent2)
      , materials_raw(area, texture::raw_mat_none)
    {}
    Texture(int tex_rows, int tex_cols)
      : size({ tex_rows, tex_cols })
      , area(tex_rows*tex_cols)
      , glyphs(area, ' ')
      , fg_colors(area, Color16::Default)
      , bg_colors(area, Color16::Transparent2)
      , materials_raw(area, texture::raw_mat_none)
    {}
    Texture(const Texture& other)
      : size(other.size)
      , area(other.area)
      , glyphs(other.glyphs)
      , fg_colors(other.fg_colors)
      , bg_colors(other.bg_colors)
      , materials_raw(other.materials_raw)
    {}
    
    void init_raw_materials(uint8_t raw_mat)
    {
      stlutils::memset(materials_raw, raw_mat);
    }
    
    inline int index(int r, int c) const noexcept
    {
      return r * size.c + c;
    }
    
    Textel operator()(int r, int c) const
    {
      if (!check_range(r, c))
        return {};
      Textel tex;
      int idx = index(r, c);
      tex.glyph = glyphs[idx];
      tex.fg_color = fg_colors[idx];
      tex.bg_color = bg_colors[idx];
      tex.mat_raw = materials_raw[idx];
      return tex;
    }
    
    Textel operator()(const RC& pos) const
    {
      return operator()(pos.r, pos.c);
    }
    
    void set_textel(int r, int c, const Textel& textel)
    {
      if (!check_range(r, c))
        return;
      int idx = index(r, c);
      glyphs[idx] = textel.glyph;
      fg_colors[idx] = textel.fg_color;
      bg_colors[idx] = textel.bg_color;
      materials_raw[idx] = textel.mat_raw;
    }
    
    void set_textel(const RC& pos, const Textel& textel)
    {
      set_textel(pos.r, pos.c, textel);
    }
    
    void set_textel_char(int r, int c, char ch)
    {
      if (!check_range(r, c))
        return;
      glyphs[index(r, c)] = ch;
    }
    
    void set_textel_char(const RC& pos, char ch)
    {
      set_textel_char(pos.r, pos.c, ch);
    }
    
    void set_textel_glyph(int r, int c, const Glyph& g)
    {
      if (!check_range(r, c))
        return;
      glyphs[index(r, c)] = g;
    }
    
    void set_textel_glyph(const RC& pos, const Glyph& g)
    {
      set_textel_glyph(pos.r, pos.c, g);
    }
    
    void set_textel_fg_color(int r, int c, Color fg_color)
    {
      if (!check_range(r, c))
        return;
      fg_colors[index(r, c)] = fg_color;
    }
    
    void set_textel_fg_color(const RC& pos, Color fg_color)
    {
      set_textel_fg_color(pos.r, pos.c, fg_color);
    }
    
    void set_textel_bg_color(int r, int c, Color bg_color)
    {
      if (!check_range(r, c))
        return;
      bg_colors[index(r, c)] = bg_color;
    }
    
    void set_textel_bg_color(const RC& pos, Color bg_color)
    {
      set_textel_bg_color(pos.r, pos.c, bg_color);
    }
    
    void set_textel_material_raw(int r, int c, uint8_t mat_raw)
    {
      if (!check_range(r, c))
        return;
      materials_raw[index(r, c)] = mat_raw;
    }
    
    void set_textel_material_raw(const RC& pos, uint8_t mat)
    {
      set_textel_material_raw(pos.r, pos.c, mat);
    }
    
    uint8_t get_textel_material_raw(int r, int c) const
    {
      if (!check_range(r, c))
        return texture::raw_mat_none;
      return materials_raw[index(r, c)];
    }
    
    uint8_t get_textel_material_raw(const RC& pos) const
    {
      return get_textel_material_raw(pos.r, pos.c);
    }
    
    void set_textel_material(int r, int c, int mat)
    {
      set_textel_material_raw(r,c, texture::encode_raw_material(mat));
    }
    
    void set_textel_material(const RC& pos, int mat)
    {
      set_textel_material(pos.r, pos.c, mat);
    }
    
    int get_textel_material(int r, int c) const
    {
      return texture::decode_raw_material(get_textel_material_raw(r, c));
    }
    
    int get_textel_material(const RC& pos) const
    {
      return get_textel_material(pos.r, pos.c);
    }
    
    // +-+
    // | |
    // +-+
    // zero_area_is_one_char = false (default):
    // r_len = 3, c_len = 3
    // zero_area_is_one_char = true:
    // r_len = 2, c_len = 2
    // So that:
    // zero_area_is_one_char = true:
    // r_len = 0, c_len = 0
    // +
    
    Texture subset(const Rectangle& bb_subset, bool zero_area_is_one_char = false)
    {
      auto bb = bb_subset; //
      if (zero_area_is_one_char)
      {
        bb.r_len++;
        bb.c_len++;
      }
      
      Texture sub_texture(bb.size());
      for (int r_src = bb.top(); r_src <= bb.bottom(); ++r_src)
      {
        if (!check_range_r(r_src))
          continue;
        auto r_dst = r_src - bb.r;
        for (int c_src = bb.left(); c_src <= bb.right(); ++c_src)
        {
          if (!check_range_c(c_src))
            continue;
          auto c_dst = c_src - bb.c;
          int idx_src = r_src * size.c + c_src;
          int idx_dst = r_dst * sub_texture.size.c + c_dst;
          sub_texture.glyphs[idx_dst] = glyphs[idx_src];
          sub_texture.fg_colors[idx_dst] = fg_colors[idx_src];
          sub_texture.bg_colors[idx_dst] = bg_colors[idx_src];
          sub_texture.materials_raw[idx_dst] = materials_raw[idx_src];
        }
      }
      
      return sub_texture;
    }
    
    bool empty() const { return size.r == 0 && size.c == 0; }
    
    void clear()
    {
      size = { -1, -1 };
      area = 0;
      glyphs.clear();
      fg_colors.clear();
      bg_colors.clear();
      materials_raw.clear();
    }
    
    bool check_range_r(int r) const noexcept
    {
      if (math::in_range(r, 0, size.r, Range::ClosedOpen))
        return true;
      return false;
    }
    
    bool check_range_c(int c) const noexcept
    {
      if (math::in_range(c, 0, size.c, Range::ClosedOpen))
        return true;
      return false;
    }
    
    bool check_range(int r, int c) const noexcept
    {
      if (!check_range_r(r))
        return false;
      if (!check_range_c(c))
        return false;
      return true;
    }
    
  };
  
}
