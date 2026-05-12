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
#include <sstream>


namespace t8
{
  
  namespace texture
  {
    static constexpr uint8_t raw_mat_none = 255;
    static constexpr int mat_none = -1;
    
    inline bool has_material_raw(uint8_t m_raw) { return m_raw != raw_mat_none; }
    //inline bool has_material(int m) { return m != mat_none; }
    
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
      return (m_raw == raw_mat_none) ? -1 : static_cast<int>(m_raw);
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
  
  enum class TxGlyphEncoding
  {
    AsciiOnly,                                   // 1 byte per cell.
    EnforceUnicodePreferredAndFallback,          // Store preferred + fallback always.
    TryUnicodePreferredAndFallbackElseAsciiOnly, // Store preferred + fallback if any preferred > 0x7F exists.
  };
  
  enum class TextureFileFormat
  {
    Auto,
    Tx,
    Ansi,
  };
  
  enum class AnsiGlyphEncoding
  {
    Auto,
    UTF8,
    CP437,
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
    
    bool load(const std::string& file_path, bool verbose)
    {
      return load(file_path, TextureFileFormat::Auto, verbose);
    }
    
    bool load(const std::string& file_path,
              TextureFileFormat format = TextureFileFormat::Auto,
              bool verbose = true,
              AnsiGlyphEncoding ansi_glyph_encoding = AnsiGlyphEncoding::Auto,
              Color ansi_default_fg = Color16::Default,
              Color ansi_default_bg = Color16::Transparent2)
    {
      auto resolved_format = format == TextureFileFormat::Auto ?
        deduce_file_format(file_path) : format;
    
      Texture parsed;
      
      bool ok = false;
      switch (resolved_format)
      {
        case TextureFileFormat::Auto:
          return false;
        case TextureFileFormat::Tx:
          ok = parsed.load_tx(file_path, verbose);
          break;
        case TextureFileFormat::Ansi:
          ok = parsed.load_ansi(file_path, verbose,
                                ansi_glyph_encoding,
                                ansi_default_fg, ansi_default_bg);
          break;
      }
      
      if (!ok)
        return false;
      
      *this = std::move(parsed);
      return true;
    }
    
    bool save(const std::string& file_path, bool verbose, TxGlyphEncoding encoding_mode)
    {
      return save(file_path, TextureFileFormat::Auto, verbose, encoding_mode);
    }
    
    bool save(const std::string& file_path,
              TextureFileFormat format = TextureFileFormat::Auto,
              bool verbose = true,
              TxGlyphEncoding encoding_mode = TxGlyphEncoding::AsciiOnly,
              AnsiGlyphEncoding ansi_glyph_encoding = AnsiGlyphEncoding::Auto,
              Color ansi_default_fg = Color16::Default,
              Color ansi_default_bg = Color16::Transparent2)
    {
      auto resolved_format = format == TextureFileFormat::Auto ?
        deduce_file_format(file_path) : format;
    
      switch (resolved_format)
      {
        case TextureFileFormat::Auto:
          return false;
        case TextureFileFormat::Tx:
          return save_tx(file_path, encoding_mode);
        case TextureFileFormat::Ansi:
          return save_ansi(file_path, verbose,
                           ansi_glyph_encoding,
                           ansi_default_fg,
                           ansi_default_bg);
      }
      
      return false;
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
    
  private:
    bool has_8bit_colors() const
    {
      for (auto col : fg_colors)
        if (!col.is_color16())
          return true;
      for (auto col : bg_colors)
        if (!col.is_color16())
          return true;
      return false;
    }
    
    bool has_empty_materials() const
    {
      for (auto raw_mat : materials_raw)
        if (raw_mat == texture::raw_mat_none)
          return true;
      return false;
    }
    
    bool has_non_ascii_glyphs() const
    {
      for (const auto& g : glyphs)
        if (g.preferred > 0x7F)
          return true;
      return false;
    }
  
    int compute_minimal_version(int max_ver = math::get_max<int>()) const
    {
      int minimal_ver = 10;
      if (minimal_ver < 20 && 20 <= max_ver && has_8bit_colors())
        minimal_ver = 20;
      // VER 2.1: material "none" is serialized as '-' (stored internally as 255).
      if (minimal_ver < 21 && 21 <= max_ver && has_empty_materials())
        minimal_ver = 21;
      if (minimal_ver < 30 && 30 <= max_ver && has_non_ascii_glyphs())
        minimal_ver = 30;
      return minimal_ver;
    }
    
    // Retrieves file extennsion in lower case.
    static std::string get_file_ext(const std::string& file_path)
    {
      return str::to_lower(folder::split_filename_ext(file_path).second);
    }
    
    static TextureFileFormat deduce_file_format(const std::string& file_path)
    {
      auto ext = get_file_ext(file_path);
      if (ext == "ans" || ext == "ansi" || ext == "utf8ans" || ext == "txt")
        return TextureFileFormat::Ansi;
      if (ext == "tx")
        return TextureFileFormat::Tx;
      return TextureFileFormat::Auto;
    }
    
    static char ansi_fallback_for_unicode(char32_t cp)
    {
      switch (cp)
      {
          // Blocks and shades.
        case 0x2588: return '#'; // █ Full block
        case 0x2589: return '#'; // ▉ Left seven eighths block
        case 0x258A: return '#'; // ▊ Left three quarters block
        case 0x258B: return '#'; // ▋ Left five eighths block
        case 0x258C: return '|'; // ▌ Left half block
        case 0x258D: return '|'; // ▍ Left three eighths block
        case 0x258E: return '|'; // ▎ Left one quarter block
        case 0x258F: return '|'; // ▏ Left one eighth block
          
        case 0x2590: return '|'; // ▐ Right half block
        case 0x2591: return '.'; // ░ Light shade
        case 0x2592: return ':'; // ▒ Medium shade
        case 0x2593: return '#'; // ▓ Dark shade
          
        case 0x2580: return '-'; // ▀ Upper half block
        case 0x2584: return '_'; // ▄ Lower half block
          
          // Light box drawing.
        case 0x2500: return '-'; // ─
        case 0x2502: return '|'; // │
        case 0x250C: return '+'; // ┌
        case 0x2510: return '+'; // ┐
        case 0x2514: return '+'; // └
        case 0x2518: return '+'; // ┘
        case 0x251C: return '+'; // ├
        case 0x2524: return '+'; // ┤
        case 0x252C: return '+'; // ┬
        case 0x2534: return '+'; // ┴
        case 0x253C: return '+'; // ┼
          
          // Heavy box drawing.
        case 0x2501: return '-'; // ━
        case 0x2503: return '|'; // ┃
        case 0x250F: return '+'; // ┏
        case 0x2513: return '+'; // ┓
        case 0x2517: return '+'; // ┗
        case 0x251B: return '+'; // ┛
        case 0x2523: return '+'; // ┣
        case 0x252B: return '+'; // ┫
        case 0x2533: return '+'; // ┳
        case 0x253B: return '+'; // ┻
        case 0x254B: return '+'; // ╋
          
          // Double box drawing.
        case 0x2550: return '='; // ═
        case 0x2551: return '|'; // ║
        case 0x2552: return '+'; // ╒
        case 0x2553: return '+'; // ╓
        case 0x2554: return '+'; // ╔
        case 0x2555: return '+'; // ╕
        case 0x2556: return '+'; // ╖
        case 0x2557: return '+'; // ╗
        case 0x2558: return '+'; // ╘
        case 0x2559: return '+'; // ╙
        case 0x255A: return '+'; // ╚
        case 0x255B: return '+'; // ╛
        case 0x255C: return '+'; // ╜
        case 0x255D: return '+'; // ╝
        case 0x255E: return '+'; // ╞
        case 0x255F: return '+'; // ╟
        case 0x2560: return '+'; // ╠
        case 0x2561: return '+'; // ╡
        case 0x2562: return '+'; // ╢
        case 0x2563: return '+'; // ╣
        case 0x2564: return '+'; // ╤
        case 0x2565: return '+'; // ╥
        case 0x2566: return '+'; // ╦
        case 0x2567: return '+'; // ╧
        case 0x2568: return '+'; // ╨
        case 0x2569: return '+'; // ╩
        case 0x256A: return '+'; // ╪
        case 0x256B: return '+'; // ╫
        case 0x256C: return '+'; // ╬
          
          // Common symbols.
        case 0x00A0: return ' '; // Non-breaking space
        case 0x2022: return '*'; // • Bullet
        case 0x00B7: return '.'; // · Middle dot
        case 0x2219: return '.'; // ∙ Bullet operator
        case 0x25A0: return '#'; // ■ Black square
        case 0x25AA: return '#'; // ▪ Black small square
        case 0x25AB: return '#'; // ▫ White small square
        case 0x00AC: return '-'; // ¬ Not sign
        case 0x00B0: return 'o'; // ° Degree sign
        case 0x00B1: return '+'; // ± Plus-minus sign
        case 0x00B2: return '2'; // ² Superscript two
        case 0x00BC: return '1'; // ¼ Vulgar fraction one quarter
        case 0x00BD: return '1'; // ½ Vulgar fraction one half
        case 0x00D7: return 'x'; // × Multiplication sign
        case 0x00F7: return '/'; // ÷ Division sign
        case 0x0192: return 'f'; // ƒ Latin small letter f with hook
        case 0x207F: return 'n'; // ⁿ Superscript latin small letter n
        case 0x221A: return 'v'; // √ Square root
        case 0x221E: return '8'; // ∞ Infinity
        case 0x2229: return 'n'; // ∩ Intersection
        case 0x2248: return '~'; // ≈ Almost equal to
        case 0x2261: return '='; // ≡ Identical to
        case 0x2264: return '<'; // ≤ Less-than or equal to
        case 0x2265: return '>'; // ≥ Greater-than or equal to
        case 0x2310: return '-'; // ⌐ Reversed not sign
        case 0x2320: return '('; // ⌠ Top half integral
        case 0x2321: return ')'; // ⌡ Bottom half integral
          
          // CP437 accented letters, approximated for ASCII fallback mode.
        case 0x00AA: return 'a'; // ª Feminine ordinal indicator
        case 0x00BA: return 'o'; // º Masculine ordinal indicator
        case 0x00A1: return '!'; // ¡ Inverted exclamation mark
        case 0x00A3: return 'L'; // £ Pound sign
        case 0x00AB: return '<'; // « Left-pointing double angle quotation mark
        case 0x00BB: return '>'; // » Right-pointing double angle quotation mark
        case 0x00BF: return '?'; // ¿ Inverted question mark
        case 0x00B5: return 'u'; // µ Micro sign
        case 0x00C4: return 'A'; // Ä
        case 0x00C5: return 'A'; // Å
        case 0x00C6: return 'A'; // Æ
        case 0x00C7: return 'C'; // Ç
        case 0x00C9: return 'E'; // É
        case 0x00D1: return 'N'; // Ñ
        case 0x00D6: return 'O'; // Ö
        case 0x00D8: return 'O'; // Ø
        case 0x00DC: return 'U'; // Ü
        case 0x00DF: return 'B'; // ß
        case 0x00E0: return 'a'; // à
        case 0x00E1: return 'a'; // á
        case 0x00E2: return 'a'; // â
        case 0x00E4: return 'a'; // ä
        case 0x00E5: return 'a'; // å
        case 0x00E6: return 'a'; // æ
        case 0x00E7: return 'c'; // ç
        case 0x00E8: return 'e'; // è
        case 0x00E9: return 'e'; // é
        case 0x00EA: return 'e'; // ê
        case 0x00EB: return 'e'; // ë
        case 0x00EC: return 'i'; // ì
        case 0x00ED: return 'i'; // í
        case 0x00EE: return 'i'; // î
        case 0x00EF: return 'i'; // ï
        case 0x00F1: return 'n'; // ñ
        case 0x00F2: return 'o'; // ò
        case 0x00F3: return 'o'; // ó
        case 0x00F4: return 'o'; // ô
        case 0x00F6: return 'o'; // ö
        case 0x00F8: return 'o'; // ø
        case 0x00F9: return 'u'; // ù
        case 0x00FA: return 'u'; // ú
        case 0x00FB: return 'u'; // û
        case 0x00FC: return 'u'; // ü
        case 0x00FF: return 'y'; // ÿ
          
          // CP437 Greek/math letters, approximated for ASCII fallback mode.
        case 0x0393: return 'G'; // Γ
        case 0x0398: return 'O'; // Θ
        case 0x03A3: return 'S'; // Σ
        case 0x03A6: return 'O'; // Φ
        case 0x03A9: return 'O'; // Ω
        case 0x03B1: return 'a'; // α
        case 0x03B4: return 'd'; // δ
        case 0x03B5: return 'e'; // ε
        case 0x03C0: return 'p'; // π
        case 0x03C3: return 's'; // σ
        case 0x03C4: return 't'; // τ
        case 0x03C6: return 'o'; // φ
        
          // Special glyphs.
        case 0xFFFD: return '?'; // �
      }
      return Glyph::none;
    }
    
    static bool create_glyph_from_ansi(char32_t ch32, char fb, Glyph& g, bool verbose = true)
    {
      if (ch32 <= 0x7F)
        g = Glyph { ch32 };
      else
      {
        if (fb != Glyph::none && static_cast<unsigned char>(fb) > 0x7F)
        {
          if (verbose)
            std::cerr << "ERROR in Texture::create_glyph_from_ansi() : Encountered a malformed fallback char.\n";
          return false;
        }
        char fallback = fb != Glyph::none ? fb : ansi_fallback_for_unicode(ch32);
        if (fallback == Glyph::none)
        {
          if (verbose)
          {
            std::cerr << "ERROR in Texture::create_glyph_from_ansi() : Missing fallback for Unicode glyph: U+"
                      << std::hex << std::uppercase << static_cast<uint32_t>(ch32)
                      << std::dec << std::nouppercase << ".\n";
          }
          return false;
        }
        
        g = Glyph { ch32, fallback };
      }
      return true;
    }
    
    // BOM (Byte Order Mark).
    // UTF-8 : EF BB BF
    static bool has_utf8_bom(const std::vector<std::string>& lines)
    {
      if (!lines.empty() && lines[0].size() >= 3)
      {
        const unsigned char b0 = static_cast<unsigned char>(lines[0][0]);
        const unsigned char b1 = static_cast<unsigned char>(lines[0][1]);
        const unsigned char b2 = static_cast<unsigned char>(lines[0][2]);
        
        if (b0 == 0xEF && b1 == 0xBB && b2 == 0xBF)
          return true;
      }
      return false;
    }
    
    static void strip_utf8_bom(std::vector<std::string>& lines)
    {
      lines[0].erase(0, 3);
    }
    
    static void add_utf8_bom(std::vector<std::string>& lines)
    {
      const char b0 = static_cast<char>(0xEF);
      const char b1 = static_cast<char>(0xBB);
      const char b2 = static_cast<char>(0xBF);
      std::string bstr;
      bstr.push_back(b0);
      bstr.push_back(b1);
      bstr.push_back(b2);
      
      if (lines.empty())
        lines.emplace_back().insert(0, bstr);
      else
        lines[0].insert(0, bstr);
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
    
    bool load_tx(const std::string& file_path, bool verbose = true)
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
                  std::cerr << "ERROR in Texture::load_tx() : Incompatible texture version: version = " + tokens[1] << '\n';
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
              materials_raw.resize(area, texture::raw_mat_none);
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
            std::cerr << "ERROR in Texture::load_tx() : Incorrect number of header lines.\n";
          }
        }
        else if (section == 1)
        {
          if (!l.empty())
          {
            int l_idx = 0;
            for (int c = 0; c < size.c; ++c)
              glyphs[index(r, c)].parse(l, l_idx, ver <= 21, verbose);
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
              fg_colors[index(r, c)].parse(l, l_idx, true, verbose);
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
              bg_colors[index(r, c)].parse(l, l_idx, true, verbose);
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
              materials_raw[index(r, c)] = texture::str_to_raw_mat(l, l_idx);
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
    
    bool save_tx(const std::string& file_path, TxGlyphEncoding encoding_mode = TxGlyphEncoding::AsciiOnly)
    {
      std::vector<std::string> lines;
      
      // Set the lowest supported version.
      ver = compute_minimal_version(21);
      
      bool unicode = has_non_ascii_glyphs();
      switch (encoding_mode)
      {
        case TxGlyphEncoding::AsciiOnly:
          break;
        case TxGlyphEncoding::TryUnicodePreferredAndFallbackElseAsciiOnly:
          if (!unicode)
            encoding_mode = TxGlyphEncoding::AsciiOnly;
          else
            ver = 30;
          break;
        case TxGlyphEncoding::EnforceUnicodePreferredAndFallback:
          ver = 30;
          break;
      }
      
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
          curr_line += glyphs[index(r, c)].str(encoding_mode == TxGlyphEncoding::AsciiOnly);
        lines.emplace_back(curr_line);
      }
      lines.emplace_back("");
      for (int r = 0; r < size.r; ++r)
      {
        std::string curr_line;
        for (int c = 0; c < size.c; ++c)
          curr_line += fg_colors[index(r, c)].str(true);
        lines.emplace_back(curr_line);
      }
      lines.emplace_back("");
      for (int r = 0; r < size.r; ++r)
      {
        std::string curr_line;
        for (int c = 0; c < size.c; ++c)
          curr_line += bg_colors[index(r, c)].str(true);
        lines.emplace_back(curr_line);
      }
      lines.emplace_back("");
      for (int r = 0; r < size.r; ++r)
      {
        std::string curr_line;
        for (int c = 0; c < size.c; ++c)
          curr_line += texture::raw_mat_to_str(materials_raw[index(r, c)]);
        lines.emplace_back(curr_line);
      }
      
      return TextIO::write_file(file_path, lines);
    }
    
    bool load_ansi(const std::string& file_path,
                   bool verbose = true,
                   AnsiGlyphEncoding glyph_encoding = AnsiGlyphEncoding::Auto,
                   Color ansi_default_fg = Color16::Default,
                   Color ansi_default_bg = Color16::Transparent2)
    {
      std::vector<std::string> lines;
      bool ret = TextIO::read_file(file_path, lines);
      if (!ret)
        return false;
      
      // Cases:
      // :w (warning), :c (correct).
      // *.ans, *.ansi (no BOM): Auto->CP437:c, UTF8:w, CP437:c
      // *.ans, *.ansi (has BOM): Auto->UTF8:c, UTF8:c, CP437:w
      // *.utf8ans (no BOM): Auto->UTF8:c, UTF8:c, CP437:w
      // *.utf8ans (has BOM): Auto->UTF8:c, UTF8:c, CP437:w (via BOM/CP437 conflict, not extension).
      auto ext = get_file_ext(file_path);
      bool utf8_bom = has_utf8_bom(lines);
      if (utf8_bom)
      {
        strip_utf8_bom(lines);
        
        if (glyph_encoding == AnsiGlyphEncoding::Auto)
          glyph_encoding = AnsiGlyphEncoding::UTF8;
        else if (glyph_encoding == AnsiGlyphEncoding::CP437 && verbose)
          std::cerr << "WARNING in Texture::load_ansi() : Attempting to load a UTF-8 encoded ANSI file with CP437 encoding!\n";
      }
      
      if (glyph_encoding == AnsiGlyphEncoding::Auto)
      {
        if (ext == "utf8ans")
          glyph_encoding = AnsiGlyphEncoding::UTF8;
        else // *.ans, *.ansi.
          glyph_encoding = AnsiGlyphEncoding::CP437;
      }
      else if (glyph_encoding == AnsiGlyphEncoding::UTF8 && !utf8_bom)
      {
        if (verbose && (ext == "ans" || ext == "ansi" || ext == "txt"))
          std::cerr << "WARNING in Texture::load_ansi() : Attempting to load an ANSI file in UTF-8 encoding without UTF-8 BOM!\n";
      }
      else if (glyph_encoding == AnsiGlyphEncoding::CP437 && !utf8_bom)
      {
        if (verbose && ext == "utf8ans")
          std::cerr << "WARNING in Texture::load_ansi() : Attempting to load a UTF-8 ANSI (*.utf8ans) file in CP437 encoding!\n";
      }
      
      // Normalizes DOS/Windows CRLF lines to plain content lines.
      for (auto& line : lines)
        if (!line.empty() && line.back() == '\r')
          line.pop_back();
      
      const int Nl = stlutils::sizeI(lines);
      for (int i = 0; i < Nl; ++i)
      {
        auto eof_pos = lines[i].find('\x1A');
        if (eof_pos != std::string::npos)
        {
          lines[i].erase(eof_pos);
          lines.resize(lines[i].empty() ? i : i + 1);
          break;
        }
      }
      
      const std::string empty_str_row;
      
      std::vector<std::string> fb_lines;
      auto fb_filepath = folder::join_filename_ext({ file_path, "fb" });
      TextIO::read_file(fb_filepath, fb_lines, verbose ? 2 : 1);
      
      std::vector<std::string> mat_lines;
      auto mat_filepath = folder::join_filename_ext({ file_path, "mat" });
      TextIO::read_file(mat_filepath, mat_lines, verbose ? 2 : 1);
        
      struct Cell
      {
        Glyph glyph;
        Color fg = Color16::Default;
        Color bg = Color16::Transparent2;
        uint8_t mat_raw = texture::raw_mat_none;
      };
      
      std::vector<std::vector<Cell>> rows;
      
      Color fg = ansi_default_fg;
      Color bg = ansi_default_bg;
      constexpr int ansi_terminal_width = 80;
      const auto num_long_lines = static_cast<int>(stlutils::count_if(lines,
        [](const auto& line) { return ansi_terminal_width < str::lenI(line); }));
      const bool ansi_auto_wrap = num_long_lines == 1 && stlutils::sizeI(lines) <= 2;
      
      auto f_make_blank_cell = [&]()
      {
        Cell cell;
        cell.glyph = Glyph { U' ' };
        cell.fg = ansi_default_fg;
        cell.bg = ansi_default_bg;
        return cell;
      };
      
      auto f_ensure_cursor_cell = [&](int r, int c)
      {
        while (stlutils::sizeI(rows) <= r)
          rows.emplace_back();
        
        auto& row = rows[r];
        while (stlutils::sizeI(row) <= c)
          row.emplace_back(f_make_blank_cell());
      };
      
      int cursor_r = 0;
      int cursor_c = 0;
      int saved_cursor_r = 0;
      int saved_cursor_c = 0;
      bool has_saved_cursor = false;
      bool bright_fg = false;
      auto f_wrap_cursor = [&]()
      {
        if (!ansi_auto_wrap)
          return;
        
        if (cursor_c < ansi_terminal_width)
          return;
        
        cursor_r += cursor_c / ansi_terminal_width;
        cursor_c = cursor_c % ansi_terminal_width;
      };
      
      int input_r = 0;
      const int num_input_rows = stlutils::sizeI(lines);
      for (const auto& line : lines)
      {
        const auto& fb_row = input_r < stlutils::sizeI(fb_lines) ? fb_lines[input_r] : empty_str_row;
        const auto& mat_row = input_r < stlutils::sizeI(mat_lines) ? mat_lines[input_r] : empty_str_row;
        
        size_t byte_idx = 0;
        char32_t ch32 = utf8::none;
        int fb_pos = 0;
        int mat_pos = 0;
        
        int len_line = str::lenI(line);
        for (int i = 0; i < len_line; )
        {
          byte_idx = static_cast<size_t>(i);
          
          if (line[i] == '\033')
          {
            std::vector<int> params;
            int next = i;
            if (ansi::parse_ansi_sgr_params(line, next, params))
            {
              ansi::apply_ansi_sgr_params(params, fg, bg,
                                          bright_fg,
                                          ansi_default_fg,
                                          ansi_default_bg);
              i = next;
              continue;
            }
            
            char cursor_dir = '\0';
            int cursor_count = 0;
            next = i;
            if (ansi::parse_ansi_cursor_move(line, next, cursor_dir, cursor_count))
            {
              switch (cursor_dir)
              {
                case 'A':
                  cursor_r = std::max(0, cursor_r - cursor_count);
                  break;
                case 'B':
                  cursor_r += cursor_count;
                  break;
                case 'C':
                  cursor_c += cursor_count;
                  f_wrap_cursor();
                  break;
                case 'D':
                  cursor_c = std::max(0, cursor_c - cursor_count);
                  break;
              }
              
              i = next;
              continue;
            }
            
            char erase_target = '\0';
            int erase_mode = 0;
            next = i;
            if (ansi::parse_ansi_erase(line, next, erase_target, erase_mode))
            {
              if (erase_target == 'J' && erase_mode == 2)
              {
                rows.clear();
                cursor_r = 0;
                cursor_c = 0;
                has_saved_cursor = false;
              }
              else if (erase_target == 'K' && erase_mode == 2)
              {
                while (stlutils::sizeI(rows) <= cursor_r)
                  rows.emplace_back();
                rows[cursor_r].resize(cursor_c);
              }
              
              i = next;
              continue;
            }
            
            char cursor_save_restore = '\0';
            next = i;
            if (ansi::parse_ansi_cursor_save_restore(line, next, cursor_save_restore))
            {
              if (cursor_save_restore == 's')
              {
                saved_cursor_r = cursor_r;
                saved_cursor_c = cursor_c;
                has_saved_cursor = true;
              }
              else if (has_saved_cursor)
              {
                cursor_r = saved_cursor_r;
                cursor_c = saved_cursor_c;
              }
              
              i = next;
              continue;
            }
            
            int cursor_row = 1;
            int cursor_col = 1;
            next = i;
            if (ansi::parse_ansi_cursor_position(line, next, cursor_row, cursor_col))
            {
              cursor_r = cursor_row - 1;
              cursor_c = cursor_col - 1;
              i = next;
              continue;
            }
            
            if (verbose)
            {
              std::cerr << "ERROR in Texture::load_ansi() : Unsupported ANSI escape sequence";
              if (i + 1 < len_line && line[i + 1] == '[')
              {
                std::cerr << ": \\033[";
                for (int j = i + 2; j < len_line; ++j)
                {
                  char ch = line[j];
                  if (term::is_printable_ascii(ch))
                    std::cerr << ch;
                  else
                    std::cerr << "0x" << str::int2hex(ch);
                  
                  if ('@' <= ch && ch <= '~')
                    break;
                }
                std::cerr << ".";
              }
              std::cerr << "\n";
            }
            return false;
          }
          
          bool decoded = false;
          
          unsigned char b = static_cast<unsigned char>(line[i]);
          if (b < 0x20)
          {
            ch32 = U' ';
            ++i;
            decoded = true;
          }
          else if (glyph_encoding == AnsiGlyphEncoding::UTF8)
          {
            decoded = utf8::decode_next_utf8_char32(line, ch32, byte_idx);
            i = static_cast<int>(byte_idx);
          }
          else if (glyph_encoding == AnsiGlyphEncoding::CP437)
          {
            unsigned char b = static_cast<unsigned char>(line[i]);
            auto cp = utf8::cp437_to_unicode(b);
            if (cp.has_value())
            {
              ch32 = cp.value();
              ++i;
              decoded = true;
            }
          }
          
          if (!decoded)
            return false;
          
          Cell cell;
          const auto fb = fb_pos < str::lenI(fb_row) ? fb_row[fb_pos] : Glyph::none;
          ++fb_pos;
          if (!create_glyph_from_ansi(ch32, fb, cell.glyph, verbose))
          {
            if (verbose)
              std::cerr << "ERROR in Texture::load_ansi() : Unable to create glyph object from ANSI file unicode bytes.\n";
            return false;
          }
          cell.mat_raw = texture::str_to_raw_mat(mat_row, mat_pos);
          cell.fg = fg;
          cell.bg = bg;
          
          f_ensure_cursor_cell(cursor_r, cursor_c);
          rows[cursor_r][cursor_c] = cell;
          ++cursor_c;
          f_wrap_cursor();
        }
        
        ++input_r;
        if (input_r < num_input_rows)
        {
          ++cursor_r;
          cursor_c = 0;
        }
      }
      
      int num_rows = stlutils::sizeI(rows);
      int num_cols = 0;
      for (const auto& row : rows)
        math::maximize(num_cols, stlutils::sizeI(row));

      size = { num_rows, num_cols };
      area = size.r * size.c;
      glyphs.assign(area, ' ');
      fg_colors.assign(area, Color16::Default);
      bg_colors.assign(area, Color16::Transparent2);
      materials_raw.assign(area, texture::raw_mat_none);
      
      for (int r = 0; r < num_rows; ++r)
      {
        int nc = stlutils::sizeI(rows[r]);
        for (int c = 0; c < nc; ++c)
        {
          int idx = index(r, c);
          glyphs[idx] = rows[r][c].glyph;
          fg_colors[idx] = rows[r][c].fg;
          bg_colors[idx] = rows[r][c].bg;
          materials_raw[idx] = rows[r][c].mat_raw;
        }
      }
      
      return true;
    }
    
    bool save_ansi(const std::string& file_path,
                   bool verbose = true,
                   AnsiGlyphEncoding ansi_glyph_encoding = AnsiGlyphEncoding::Auto,
                   Color ansi_default_fg = Color16::Default,
                   Color ansi_default_bg = Color16::Transparent2)
    {
      std::vector<std::string> lines;
      
      auto ext = get_file_ext(file_path);
      // Cases:
      // :w (warning), :c (correct).
      // *.ans, *.ansi: Auto->CP437, UTF8->BOM:c, CP437:c
      // *.utf8ans: Auto->UTF8, UTF8:c, CP437:w
      switch (ansi_glyph_encoding)
      {
        case AnsiGlyphEncoding::Auto:
          if (ext == "utf8ans")
            ansi_glyph_encoding = AnsiGlyphEncoding::UTF8;
          else
            ansi_glyph_encoding = AnsiGlyphEncoding::CP437;
          break;
        case AnsiGlyphEncoding::UTF8:
          if (ext == "ans" || ext == "ansi" || ext == "txt")
            add_utf8_bom(lines);
          break;
        case AnsiGlyphEncoding::CP437:
          if (ext == "utf8ans" && verbose)
            std::cerr << "WARNING in Texture::save_ansi() : Attempting to save a UTF-8 ANSI (*.utf8ans) file in CP437 encoding!\n";
          break;
      }
      
      std::vector<std::string> fb_lines;
      auto fb_filepath = folder::join_filename_ext({ file_path, "fb" });
      
      std::vector<std::string> mat_lines;
      auto mat_filepath = folder::join_filename_ext({ file_path, "mat" });
      
      Color curr_fg = ansi_default_fg;
      Color curr_bg = ansi_default_bg;
      
      for (int r = 0; r < size.r; ++r)
      {
        auto& line = lines.emplace_back();
        auto& fb_line = fb_lines.emplace_back();
        auto& mat_line = mat_lines.emplace_back();
        
        for (int c = 0; c < size.c; ++c)
        {
          auto idx = index(r, c);
          auto fg = fg_colors[idx];
          auto bg = bg_colors[idx];
          
          if (fg != curr_fg || bg != curr_bg)
          {
            line += ansi::colors_to_ansi_sgr_string(fg, bg, ansi_default_fg, ansi_default_bg);
            curr_fg = fg;
            curr_bg = bg;
          }
          
          const auto& g = glyphs[idx];
          if (ansi_glyph_encoding == AnsiGlyphEncoding::UTF8)
            line += g.encode_single_width_glyph<char32_t>();
          else if (ansi_glyph_encoding == AnsiGlyphEncoding::CP437)
          {
            auto ch_pref = static_cast<char>(static_cast<unsigned char>(g.preferred));
            auto ch_fb = g.fallback;
            auto cp = utf8::unicode_to_cp437(g.preferred);
            if (cp.has_value())
              line.push_back(static_cast<char>(cp.value()));
            else if (!g.empty() && term::is_printable_ascii(ch_pref))
              line.push_back(ch_pref);
            else if (!g.empty_fallback() && term::is_printable_ascii(ch_fb))
              line.push_back(ch_fb);
            else if (verbose)
              std::cerr << "ERROR in Texture::save_ansi() : Unable to encode glyph " << g.str(true) << " in CP437 encoding!\n";
          }
          
          fb_line.push_back(g.fallback);
          mat_line += texture::raw_mat_to_str(materials_raw[idx]);
        }
        
        line += "\033[0m";
        
        curr_fg = ansi_default_fg;
        curr_bg = ansi_default_bg;
      }
      
      auto ret_fb = TextIO::write_file(fb_filepath, fb_lines, verbose ? 2 : 1);
      auto ret_mat = TextIO::write_file(mat_filepath, mat_lines, verbose ? 2 : 1);
      auto ret_main = TextIO::write_file(file_path, lines);
      return ret_fb && ret_mat && ret_main;
    }
  };

}
