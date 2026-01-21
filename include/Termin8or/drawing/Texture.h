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
#include <Core/TextIO.h>
#include <Core/StringHelper.h>
#include <sstream>


namespace t8
{
  
  namespace texture
  {
    static constexpr uint8_t mat_none = 255;
    
    inline bool has_material(uint8_t m) { return m != mat_none; }
    
    inline uint8_t encode_material(int m)
    {
      if (m < 0) return mat_none;
      if (m > 254) return 254; // Or maybe assert instead?
      return static_cast<uint8_t>(m);
    }
    
    inline int decode_material(uint8_t m)
    {
      return (m == mat_none) ? -1 : static_cast<int>(m);
    }
    
    std::string mat_to_str(uint8_t mat)
    {
      constexpr uint8_t span09 = 1 + (9 - 0);     // 10, 0-9
      constexpr uint8_t spanAZ = 1 + ('Z' - 'A'); // 26, A-Z, a-z
      constexpr uint8_t page_span = span09 + 2*spanAZ; // 62
      if (mat >= mat_none)
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
    
    uint8_t str_to_mat(std::string_view sv, int& pos)
    {
      constexpr uint8_t span09 = 1 + (9 - 0);     // 10, 0-9
      constexpr uint8_t spanAZ = 1 + ('Z' - 'A'); // 26, A-Z, a-z
      constexpr uint8_t page_span = span09 + 2*spanAZ; // 62
      if (sv.length() <= static_cast<size_t>(pos))
        return mat_none;
        
      //char ch_prefix = 0;
      char ch = sv[pos];
      
      if (ch == '-')
      {
        pos++;
        return texture::mat_none;
      }
      
      static constexpr std::array<char, 4> prefix_list { '.', ',', ':', ';' };
      
      auto idx_prefix = stlutils::find_idx(prefix_list, ch);
      int dp = 1;
      int mat_page_offs = 0;
      if (0 <= idx_prefix)
      {
        if (sv.length() <= static_cast<size_t>(pos + 1))
          return mat_none;
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
          if (mat >= mat_none)
            material = mat_none;
          else
            material = static_cast<uint8_t>(mat);
          return true;
        }
        return false;
      };
      
      uint8_t mat = mat_none;
      if (f_check_range('0', '9', mat_page_offs, mat))
        return mat;
      if (f_check_range('A', 'Z', mat_page_offs + span09, mat))
        return mat;
      if (f_check_range('a', 'z', mat_page_offs + span09 + spanAZ, mat))
        return mat;
      
      return mat_none;
    }
  }

  // size.r = 1, size.c = 1 yields a 1x1 texture.
  struct Textel
  {
    Glyph glyph = ' ';
    Color fg_color = Color16::Default;
    Color bg_color = Color16::Transparent2;
    uint8_t mat = texture::mat_none;
    
    Style get_style() const { return { fg_color, bg_color }; }
    void set_style(const Style& style)
    {
      fg_color = style.fg_color;
      bg_color = style.bg_color;
    }
    
    template<typename CharT>
    std::string str() const
    {
      return glyph.encode_single_width_glyph<CharT>();
    }
    
    std::string mat_to_str() const { return texture::mat_to_str(mat); }
    
    bool operator==(const Textel& other) const
    {
      return this->glyph == other.glyph
        && this->fg_color == other.fg_color
        && this->bg_color == other.bg_color
        && this->mat == other.mat;
    }
  };
  
  struct Texture
  {
    enum class TxGlyphEncoding
    {
      AsciiOnly,                  // 1 byte per cell.
      UnicodePreferredAndFallback // Store preferred + fallback if any.
    };
  
    static const int compatible_version_until_and_including = 21;
    int ver = compatible_version_until_and_including;
    RC size;
    int area = 0;
    std::vector<Glyph> glyphs;
    std::vector<Color> fg_colors;
    std::vector<Color> bg_colors;
    std::vector<uint8_t> materials;
    
    Texture() = default;
    Texture(const RC& tex_size)
      : size(tex_size)
      , area(tex_size.r*tex_size.c)
      , glyphs(area, ' ')
      , fg_colors(area, Color16::Default)
      , bg_colors(area, Color16::Transparent2)
      , materials(area, texture::mat_none)
    {}
    Texture(int tex_rows, int tex_cols)
      : size({ tex_rows, tex_cols })
      , area(tex_rows*tex_cols)
      , glyphs(area, ' ')
      , fg_colors(area, Color16::Default)
      , bg_colors(area, Color16::Transparent2)
      , materials(area, texture::mat_none)
    {}
    Texture(const Texture& other)
      : size(other.size)
      , area(other.area)
      , glyphs(other.glyphs)
      , fg_colors(other.fg_colors)
      , bg_colors(other.bg_colors)
      , materials(other.materials)
    {}
    
    void init_materials(uint8_t mat)
    {
      stlutils::memset(materials, mat);
    }
    
    Textel operator()(int r, int c) const
    {
      if (!check_range(r, c))
        return {};
      Textel tex;
      int idx = r * size.c + c;
      tex.glyph = glyphs[idx];
      tex.fg_color = fg_colors[idx];
      tex.bg_color = bg_colors[idx];
      tex.mat = materials[idx];
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
      int idx = r * size.c + c;
      glyphs[idx] = textel.glyph;
      fg_colors[idx] = textel.fg_color;
      bg_colors[idx] = textel.bg_color;
      materials[idx] = textel.mat;
    }
    
    void set_textel(const RC& pos, const Textel& textel)
    {
      set_textel(pos.r, pos.c, textel);
    }
    
    void set_textel_char(int r, int c, char ch)
    {
      if (!check_range(r, c))
        return;
      int idx = r * size.c + c;
      glyphs[idx] = ch;
    }
    
    void set_textel_char(const RC& pos, char ch)
    {
      set_textel_char(pos.r, pos.c, ch);
    }
    
    void set_textel_glyph(int r, int c, const Glyph& g)
    {
      if (!check_range(r, c))
        return;
      int idx = r * size.c + c;
      glyphs[idx] = g;
    }
    
    void set_textel_glyph(const RC& pos, const Glyph& g)
    {
      set_textel_glyph(pos.r, pos.c, g);
    }
    
    void set_textel_fg_color(int r, int c, Color fg_color)
    {
      if (!check_range(r, c))
        return;
      int idx = r * size.c + c;
      fg_colors[idx] = fg_color;
    }
    
    void set_textel_fg_color(const RC& pos, Color fg_color)
    {
      set_textel_fg_color(pos.r, pos.c, fg_color);
    }
    
    void set_textel_bg_color(int r, int c, Color bg_color)
    {
      if (!check_range(r, c))
        return;
      int idx = r * size.c + c;
      bg_colors[idx] = bg_color;
    }
    
    void set_textel_bg_color(const RC& pos, Color bg_color)
    {
      set_textel_bg_color(pos.r, pos.c, bg_color);
    }
    
    void set_textel_material_raw(int r, int c, uint8_t mat)
    {
      if (!check_range(r, c))
        return;
      int idx = r * size.c + c;
      materials[idx] = mat;
    }
    
    void set_textel_material_raw(const RC& pos, uint8_t mat)
    {
      set_textel_material(pos.r, pos.c, mat);
    }
    
    uint8_t get_textel_material_raw(int r, int c) const
    {
      if (!check_range(r, c))
        return texture::mat_none;
      int idx = r * size.c + c;
      return materials[idx];
    }
    
    uint8_t get_textel_material_raw(const RC& pos) const
    {
      return get_textel_material(pos.r, pos.c);
    }
    
    void set_textel_material(int r, int c, int mat)
    {
      set_textel_material_raw(r,c, texture::encode_material(mat));
    }
    
    void set_textel_material(const RC& pos, int mat)
    {
      set_textel_material(pos.r, pos.c, mat);
    }
    
    int  get_textel_material(int r, int c) const
    {
      return texture::decode_material(get_textel_material_raw(r, c));
    }
    
    int  get_textel_material(const RC& pos) const
    {
      return get_textel_material(pos.r, pos.c);
    }
    
    // File format:
    // size, chars, fg-colors, bg-colors, materials.
    //-------------
    // 3 11
    //
    // /|||::::~~~
    // |:|^..::o~~
    // II#&7/(8---
    //
    // 5A7A4T4BBFF
    // 56633AABBFF
    // TTT5T7T5BBF
    //
    // 122238223CC
    // 1222322255C
    // 2333111CC5T
    //
    // 00001111222
    // 01030110222
    // 44544344222
    
    bool load(const std::string& file_path)
    {
      std::vector<std::string> lines;
      
      bool ret = TextIO::read_file(file_path, lines);
      if (!ret)
        return false;
      
      int section = 0;
      int r = 0;
      for (const auto& l : lines)
      {
        if (section == 0)
        {
          if (!l.empty())
          {
            if (l.starts_with("VER"))
            {
              auto tokens = str::tokenize(l, { ' ', '.' });
              if (tokens.size() >= 2)
              {
                // VER 1 : Original version. Works without this line.
                // VER 2 : Added 8-bit color support.
                // VER 2.1 : Support for -1 materials (represented as '-') and
                //   fixed missing VER string when saving.
                // Absence of VER line means it is version 1.
                int maj_ver_parsed = std::stoi(tokens[1]);
                int min_ver_parsed = tokens.size() == 3 ? std::stoi(tokens[2]) : 0;
                int ver_parsed = maj_ver_parsed*10 + min_ver_parsed;
                if (ver_parsed <= compatible_version_until_and_including)
                  ver = ver_parsed;
                else
                {
                  std::cerr << "ERROR in Texture::load() : Incompatible texture version: version = " + tokens[1] << '\n';
                  return false;
                }
              }
              else
                ver = 10; // Absence of VER line means it is version 1.0 -> 10.
            }
            else
            {
              std::istringstream iss(l);
              iss >> size.r >> size.c;
              area = size.r * size.c;
              glyphs.resize(area, ' ');
              fg_colors.resize(area, Color16::Default);
              bg_colors.resize(area, Color16::Transparent2);
              materials.resize(area, texture::mat_none);
            }
            r++;
          }
          else if (r == 1 || r == 2) // 2 if there is a VER line.
          {
            section = 1;
            r = 0;
          }
          else if (r > 2)
          {
            std::cerr << "ERROR in Texture::parse() : Incorrect number of header lines.\n";
          }
        }
        else if (section == 1)
        {
          if (!l.empty())
          {
            for (int c = 0; c < size.c; ++c)
            {
              int idx = r * size.c + c;
              glyphs[idx] = l[c];
            }
            r++;
          }
          else if (r > 0)
          {
            section = 2;
            r = 0;
          }
        }
        else if (section == 2)
        {
          if (!l.empty())
          {
            int l_idx = 0;
            for (int c = 0; c < size.c; ++c)
            {
              int idx = r * size.c + c;
              fg_colors[idx].parse(l, l_idx);
            }
            r++;
          }
          else if (r > 0)
          {
            section = 3;
            r = 0;
          }
        }
        else if (section == 3)
        {
          if (!l.empty())
          {
            int l_idx = 0;
            for (int c = 0; c < size.c; ++c)
            {
              int idx = r * size.c + c;
              bg_colors[idx].parse(l, l_idx);
            }
            r++;
          }
          else if (r > 0)
          {
            section = 4;
            r = 0;
          }
        }
        else if (section == 4)
        {
          if (!l.empty())
          {
            int l_idx = 0;
            for (int c = 0; c < size.c; ++c)
            {
              int idx = r * size.c + c;
              materials[idx] = texture::str_to_mat(l, l_idx);
            }
            r++;
          }
          else if (r > 0)
          {
            section = 5;
            r = 0;
          }
        }
      }
      return true;
    }
    
    bool save(const std::string& file_path, TxGlyphEncoding encoding_mode = TxGlyphEncoding::AsciiOnly)
    {
      std::vector<std::string> lines;
      
      // Set the lowest supported version.
      ver = compute_minimal_version();
      
      int maj_ver = ver / 10;
      int min_ver = ver % 10;
      lines.emplace_back("VER " + std::to_string(maj_ver) + "." + std::to_string(min_ver));
      
      std::ostringstream oss;
      oss << size.r << " " << size.c;
      lines.emplace_back(oss.str());
      lines.emplace_back("");
      for (int r = 0; r < size.r; ++r)
      {
        std::string curr_line;
        for (int c = 0; c < size.c; ++c)
        {
          int idx = r * size.c + c;
          curr_line += glyphs[idx].str(encoding_mode == TxGlyphEncoding::AsciiOnly);
        }
        lines.emplace_back(curr_line);
      }
      lines.emplace_back("");
      for (int r = 0; r < size.r; ++r)
      {
        std::string curr_line;
        for (int c = 0; c < size.c; ++c)
        {
          int idx = r * size.c + c;
          curr_line += fg_colors[idx].str(true);
        }
        lines.emplace_back(curr_line);
      }
      lines.emplace_back("");
      for (int r = 0; r < size.r; ++r)
      {
        std::string curr_line;
        for (int c = 0; c < size.c; ++c)
        {
          int idx = r * size.c + c;
          curr_line += bg_colors[idx].str(true);
        }
        lines.emplace_back(curr_line);
      }
      lines.emplace_back("");
      for (int r = 0; r < size.r; ++r)
      {
        std::string curr_line;
        for (int c = 0; c < size.c; ++c)
        {
          int idx = r * size.c + c;
          curr_line += texture::mat_to_str(materials[idx]);
        }
        lines.emplace_back(curr_line);
      }
      
      return TextIO::write_file(file_path, lines);
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
      for (int r = bb.top(); r <= bb.bottom(); ++r)
      {
        for (int c = bb.left(); c <= bb.right(); ++c)
        {
          int idx_from = r * size.c + c;
          int idx_to = (r - bb.r) * bb.c_len + (c - bb.c);
          sub_texture.glyphs[idx_to] = glyphs[idx_from];
          sub_texture.fg_colors[idx_to] = fg_colors[idx_from];
          sub_texture.bg_colors[idx_to] = bg_colors[idx_from];
          sub_texture.materials[idx_to] = materials[idx_from];
          
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
      materials.clear();
    }
    
  private:
    bool check_range(int r, int c) const
    {
      if (!math::in_range(r, 0, size.r, Range::ClosedOpen))
        return false;
      if (!math::in_range(c, 0, size.c, Range::ClosedOpen))
        return false;
      return true;
    }
    
    int compute_minimal_version() const
    {
      int minimal_ver = 10;
      if (minimal_ver < 20)
      {
        for (auto col : fg_colors)
          if (!col.is_color16())
          {
            minimal_ver = 20;
            break;
          }
      }
      if (minimal_ver < 20)
      {
        for (auto col : bg_colors)
          if (!col.is_color16())
          {
            minimal_ver = 20;
            break;
          }
      }
      // VER 2.1: material "none" is serialized as '-' (stored internally as 255).
      if (minimal_ver < 21)
      {
        for (auto mat : materials)
          if (mat == texture::mat_none)
          {
            minimal_ver = 21;
            break;
          }
      }
      return minimal_ver;
    }
  };

}
