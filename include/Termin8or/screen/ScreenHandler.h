#pragma once
#include "Text.h"
#include "Styles.h"
#include "../geom/Rectangle.h"
#include "../geom/RC.h"
#include "../drawing/Texture.h"
#include <Core/StringHelper.h>
#include <Core/StringBox.h>
#include <Core/StlUtils.h>
#include <Core/MathUtils.h>
#include <Core/Benchmark.h>
#include <Core/System.h>
#include <array>
#include <map>
#include <memory>

namespace t8
{
  template<int NR, int NC, typename CharT>
  class ScreenHandler;
}

namespace t8x::screen_scaling
{
  template<int NRo, int NCo, int NRi, int NCi, typename CharT>
  void resample(const t8::ScreenHandler<NRi, NCi, CharT>& sh_src,
                t8::ScreenHandler<NRo, NCo, CharT>& sh_dst);
}

namespace t8
{

  struct GlyphString
  {
    GlyphString() = default;
    GlyphString(std::string_view sv)
    {
      glyph_vector.reserve(sv.size());
      for (unsigned char c : sv)
        glyph_vector.emplace_back(c, c);
    }
    GlyphString(const std::string& str)
    {
      glyph_vector.reserve(str.size());
      for (unsigned char c : str)
        glyph_vector.emplace_back(c, c);
    }
    GlyphString(const Glyph& glyph)
    {
      glyph_vector.emplace_back(glyph);
    }
    
    template<int NR, int NC, typename CharT>
    std::string encode(ScreenHandler<NR, NC, CharT>& sh) const
    {
      std::string str;
      for (const auto& g : glyph_vector)
        str += sh.encode_single_width_glyph(g);
      return str;
    }
    
    Glyph& operator[](int idx) { return glyph_vector[idx]; }
    const Glyph& operator[](int idx) const { return glyph_vector[idx]; }
  
    std::vector<Glyph> glyph_vector;
  };
  
  namespace literals
  {
    // "_gs" produces an ASCII part (no CharT, no ScreenHandler needed).
    inline GlyphString operator"" _gs(const char* s, size_t n)
    {
      GlyphString gs;
      gs.glyph_vector.reserve(n);
      for (size_t i = 0; i < n; ++i)
      {
        auto b = static_cast<unsigned char>(s[i]);
        gs.glyph_vector.emplace_back(b, b);
      }
      return gs;
    }
  }
  
  namespace operators
  {
    inline GlyphString operator+(GlyphString lhs, const GlyphString& rhs)
    {
      lhs.glyph_vector.reserve(lhs.glyph_vector.size() + rhs.glyph_vector.size());
      for (const auto& g : rhs.glyph_vector)
        lhs.glyph_vector.emplace_back(g);
      return lhs;
    }
  
    // Append ASCII.
    inline GlyphString operator+(GlyphString lhs, std::string_view rhs)
    {
      lhs.glyph_vector.reserve(lhs.glyph_vector.size() + rhs.size());
      for (unsigned char c : rhs)
        lhs.glyph_vector.emplace_back(c, c);
      return lhs;
    }
    
    // Append char.
    inline GlyphString operator+(GlyphString lhs, char rhs)
    {
      lhs.glyph_vector.reserve(lhs.glyph_vector.size() + 1);
      lhs.glyph_vector.emplace_back(rhs, rhs);
      return lhs;
    }
    
    // Append char32_t.
    inline GlyphString operator+(GlyphString lhs, char32_t rhs)
    {
      lhs.glyph_vector.reserve(lhs.glyph_vector.size() + 1);
      lhs.glyph_vector.emplace_back(rhs, rhs);
      return lhs;
    }
    
    // Append Glyph.
    inline GlyphString operator+(GlyphString lhs, const Glyph& rhs)
    {
      lhs.glyph_vector.reserve(lhs.glyph_vector.size() + 1);
      lhs.glyph_vector.emplace_back(rhs);
      return lhs;
    }
  }

  struct OrderedText
  {
    std::string str;
    int r = -1;
    int c = -1;
    int priority = 0;
    Style style;
  };
  
  struct OffscreenBuffer
  {
    Texture* buffer_texture = nullptr;
    RC buffer_screen_pos { 0, 0 }; //
    std::vector<char32_t> exclude_src_chars;
    std::vector<Color> exclude_src_fg_colors;
    std::vector<Color> exclude_src_bg_colors;
    std::vector<Color> dst_fill_bg_colors;
    std::vector<std::pair<Color, Color>> replace_src_dst_bg_colors;
  };
  
  enum class DrawPolicy { FULL, PARTIAL, THRESHOLD_SELECT, MEASURE_SELECT };
  
  // //////////////////////////////////////////////////
  
  // Bad design.
  static int num_chunks_prev = 10; // #FIXME: Magic number.
  
  template<int NR = 30, int NC = 80, typename CharT = char>
  class ScreenHandler
  {
    std::unique_ptr<Text> m_text;
    
    // Draw from top to bottom.
    std::array<CharT, NC*NR> screen_buffer, prev_screen_buffer;
    std::array<Color, NC*NR> fg_color_buffer, prev_fg_color_buffer;
    std::array<Color, NC*NR> bg_color_buffer, prev_bg_color_buffer;
    std::array<bool, NC*NR> dirty_flag_buffer;
    Color prev_clear_bg_color = Color16::Default;
    
    float dirty_fraction_threshold = 0.5f;
    
    benchmark::TicTocTimer t8_ScreenHandler_redraw_timer;
    double measured_delay_ms_full = 0;
    double measured_delay_ms_partial = 0;
    int num_frames_between_measurings = 1000;
    int frame = 0;
    int measure_mode = 0; // 0: full, 1: partial.
    int num_full_redraws = 0;
    int num_partial_redraws = 0;
    
    inline int index(int r, int c) const noexcept { return NC*r + c; }
    
    std::string color2str(Color col) const
    {
      auto col16 = col.try_get_color16();
      if (col16.has_value())
      {
        switch (col16.value())
        {
          case Color16::Transparent:  return "---:";
          case Color16::Transparent2: return "===:";
          case Color16::Default:      return "Def:";
          case Color16::Black:        return "Blk:";
          case Color16::DarkRed:      return "DRd:";
          case Color16::DarkGreen:    return "DGn:";
          case Color16::DarkYellow:   return "DYw:";
          case Color16::DarkBlue:     return "DBu:";
          case Color16::DarkMagenta:  return "DMa:";
          case Color16::DarkCyan:     return "DCn:";
          case Color16::LightGray:    return "LGy:";
          case Color16::DarkGray:     return "DGy:";
          case Color16::Red:          return "Red:";
          case Color16::Green:        return "Grn:";
          case Color16::Yellow:       return "Ylw:";
          case Color16::Blue:         return "Blu:";
          case Color16::Magenta:      return "Mga:";
          case Color16::Cyan:         return "Cyn:";
          case Color16::White:        return "Wht:";
          default:                    return "???:";
        }
      }
      auto rgb6 = col.try_get_rgb6();
      if (rgb6.has_value())
      {
        return std::to_string(static_cast<int>(rgb6.value().r)) +
          std::to_string(static_cast<int>(rgb6.value().g)) +
          std::to_string(static_cast<int>(rgb6.value().b)) + ":";
      }
      return "Err:";
    }
    
    std::vector<OrderedText> ordered_texts;
    
    void write_buffer_cell(CharT ch, int r, int c, int ci, Color fg_color, Color bg_color)
    {
      int c_tot = c + ci;
      if (c_tot >= 0 && c_tot < NC)
      {
        int idx = index(r, c_tot);
        auto& scr_ch = screen_buffer[idx];
        auto& scr_fg = fg_color_buffer[idx];
        auto& scr_bg = bg_color_buffer[idx];
        if (scr_ch == ' '
            && scr_bg == Color16::Transparent)
        {
          scr_ch = ch;
          scr_fg = fg_color;
          scr_bg = bg_color;
        }
        else if (scr_bg == Color16::Transparent2)
        {
          scr_bg = bg_color;
          if (scr_ch == ' ')
          {
            scr_ch = ch;
            scr_fg = fg_color;
          }
        }
      }
    }
    
  public:
    ScreenHandler()
      : m_text(std::make_unique<Text>())
    {}
    
    void clear()
    {
      for (auto& ch : screen_buffer)
        ch = ' ';
      for (auto& col : fg_color_buffer)
        col = Color16::Default;
      for (auto& col : bg_color_buffer)
        col = Color16::Transparent;
      for (auto& df : dirty_flag_buffer)
        df = false;
    }
    
    bool test_empty(int r, int c) const
    {
      return screen_buffer[index(r, c)] == ' ';
    }
    
    void set_force_ascii_fallback(bool force_fallback)
    {
      term::force_ascii_fallback = force_fallback;
    }
    
    bool get_force_ascii_fallback() const
    {
      return term::force_ascii_fallback;
    }
    
    void init_terminal_mode()
    {
      m_text->init_terminal_mode();
    }
    
    inline std::string encode_single_width_glyph(char32_t preferred,
                                                 char fallback = t8::Glyph::none) const
    {
      return t8::term::encode_single_width_glyph<CharT>(preferred, fallback);
    }
    
    inline std::string encode_single_width_glyph(const Glyph& glyph)
    {
      return t8::term::encode_single_width_glyph<CharT>(glyph.preferred, glyph.fallback);
    }
    
    // write_buffer using StringBox.
    void write_buffer(const str::StringBox& sb, int r, int c, Color fg_color, Color bg_color = Color16::Transparent)
    {
      auto Nr = static_cast<int>(sb.text_lines.size());
      for (int r_idx = 0; r_idx < Nr; ++r_idx)
        write_buffer(sb.text_lines[r_idx], r + r_idx, c, fg_color, bg_color);
    }
    
    void add_ordered_text(const OrderedText& text)
    {
      ordered_texts.emplace_back(text);
    }
    
    void write_buffer_ordered()
    {
      stlutils::sort(ordered_texts, [](const auto& tA, const auto& tB) { return tA.priority > tB.priority; });
      for (const auto& text : ordered_texts)
        write_buffer(text.str, text.r, text.c, text.style.fg_color, text.style.bg_color);
      // Purge the text vector.
      ordered_texts.clear();
    }
    
    void write_buffer(const Glyph& glyph, const RC& pos, const Style& style)
    {
      write_buffer(encode_single_width_glyph(glyph), pos.r, pos.c, style.fg_color, style.bg_color);
    }
    
    void write_buffer(const Glyph& glyph, const RC& pos, Color fg_color, Color bg_color = Color16::Transparent)
    {
      write_buffer(encode_single_width_glyph(glyph), pos.r, pos.c, fg_color, bg_color);
    }
    
    void write_buffer(const Glyph& glyph, int r, int c, const Style& style)
    {
      write_buffer(encode_single_width_glyph(glyph), r, c, style.fg_color, style.bg_color);
    }
    
    void write_buffer(const Glyph& glyph, int r, int c, Color fg_color, Color bg_color = Color16::Transparent)
    {
      write_buffer(encode_single_width_glyph(glyph), r, c, fg_color, bg_color);
    }
    
    void write_buffer(const GlyphString& gstr, const RC& pos, const Style& style)
    {
      write_buffer(gstr, pos.r, pos.c, style.fg_color, style.bg_color);
    }
    
    void write_buffer(const GlyphString& gstr, const RC& pos, Color fg_color, Color bg_color = Color16::Transparent)
    {
      write_buffer(gstr, pos.r, pos.c, fg_color, bg_color);
    }
    
    void write_buffer(const GlyphString& gstr, int r, int c, const Style& style)
    {
      write_buffer(gstr, r, c, style.fg_color, style.bg_color);
    }
    
    void write_buffer(const GlyphString& gstr, int r, int c, Color fg_color, Color bg_color = Color16::Transparent)
    {
      write_buffer(gstr.encode(*this), r, c, fg_color, bg_color);
    }
    
    void write_buffer(const std::string& str, const RC& pos, const Style& style)
    {
      write_buffer(str, pos.r, pos.c, style.fg_color, style.bg_color);
    }
    
    void write_buffer(const std::string& str, const RC& pos, Color fg_color, Color bg_color = Color16::Transparent)
    {
      write_buffer(str, pos.r, pos.c, fg_color, bg_color);
    }
    
    void write_buffer(const std::string& str, int r, int c, const Style& style)
    {
      write_buffer(str, r, c, style.fg_color, style.bg_color);
    }
    
    void write_buffer(const std::string& str, int r, int c, Color fg_color, Color bg_color = Color16::Transparent)
    {
      if (str.empty())
        return;
      if constexpr (std::is_same_v<CharT, char>)
      {
        if (r >= 0 && r < NR)
        {
          int n = static_cast<int>(str.size());
          for (int ci = 0; ci < n; ++ci)
            write_buffer_cell(str[ci], r, c, ci, fg_color, bg_color);
        }
      }
      else if constexpr (std::is_same_v<CharT, char32_t>)
      {
        if (r >= 0 && r < NR)
        {
          //int n = static_cast<int>(str.size());
          int ci = 0;
          size_t byte_idx = 0;
          char32_t ch32 = utf8::none;
          while (utf8::decode_next_utf8_char32(str, ch32, byte_idx))
          {
            write_buffer_cell(term::get_single_column_char32(ch32), r, c, ci, fg_color, bg_color);
            ci++;
          }
        }
      }
      else
        std::cerr << "ERROR in ScreenHandler<NR, NC, CharT>::write_buffer() : Unsupported CharT type.\n";
    }
    
    void replace_bg_color(Color from_bg_color, Color to_bg_color, Rectangle box)
    {
      for (int r = 0; r < NR; ++r)
      {
        for (int c = 0; c < NC; ++c)
        {
          if (box.is_inside(r, c))
          {
            auto& col = bg_color_buffer[index(r, c)];
            if (col == from_bg_color)
              col = to_bg_color;
          }
        }
      }
    }
    
    void replace_bg_color(Color to_bg_color, Rectangle box)
    {
      for (int r = 0; r < NR; ++r)
      {
        for (int c = 0; c < NC; ++c)
        {
          if (box.is_inside(r, c))
          {
            bg_color_buffer[index(r, c)] = to_bg_color;
          }
        }
      }
    }
    
    void replace_bg_color(Color to_bg_color)
    {
      for (int r = 0; r < NR; ++r)
      {
        for (int c = 0; c < NC; ++c)
        {
          bg_color_buffer[index(r, c)] = to_bg_color;
        }
      }
    }
    
    void replace_fg_color(Color to_fg_color, Rectangle box)
    {
      for (int r = 0; r < NR; ++r)
      {
        for (int c = 0; c < NC; ++c)
        {
          if (box.is_inside(r, c))
          {
            fg_color_buffer[index(r, c)] = to_fg_color;
          }
        }
      }
    }
    
    void replace_fg_color(Color to_fg_color)
    {
      for (int r = 0; r < NR; ++r)
      {
        for (int c = 0; c < NC; ++c)
        {
          fg_color_buffer[index(r, c)] = to_fg_color;
        }
      }
    }
    
    inline Color resolve_bg_color(Color bg_color, Color clear_bg_color)
    {
      return (bg_color == Color16::Transparent || bg_color == Color16::Transparent2) ? clear_bg_color : bg_color;
    }
    
    void diff_buffers(Color clear_bg_color)
    {
      for (int r = 0; r < NR; ++r)
      {
        for (int c = 0; c < NC; ++c)
        {
          int idx = index(r, c);
          auto bg_curr = resolve_bg_color(bg_color_buffer[idx], clear_bg_color);
          auto bg_prev = resolve_bg_color(prev_bg_color_buffer[idx], prev_clear_bg_color);
          dirty_flag_buffer[idx] =
               screen_buffer[idx] != prev_screen_buffer[idx]
            || fg_color_buffer[idx] != prev_fg_color_buffer[idx]
            || bg_curr != bg_prev;
        }
      }
    }
    
    void update_prev_buffers(Color clear_bg_color)
    {
      prev_screen_buffer = screen_buffer;
      prev_fg_color_buffer = fg_color_buffer;
      prev_bg_color_buffer = bg_color_buffer;
      prev_clear_bg_color = clear_bg_color;
    }
    
    void set_dirty_fraction_threshold(float thres)
    {
      dirty_fraction_threshold = thres;
    }
    
    void set_num_frames_between_measurings(int num_frames)
    {
      num_frames_between_measurings = num_frames;
    }
    
    int get_num_full_redraws() const { return num_full_redraws; }
    int get_num_partial_redraws() const { return num_partial_redraws; }
    
    void print_screen_buffer(Color clear_bg_color, DrawPolicy draw_policy = DrawPolicy::MEASURE_SELECT)
    {
      auto f_full_redraw = [this](Color clear_bg_color)
      {
        print_screen_buffer_full(clear_bg_color);
        num_full_redraws++;
      };
    
      auto f_partial_redraw = [this](Color clear_bg_color)
      {
        diff_buffers(clear_bg_color);
        print_screen_buffer_partial(clear_bg_color);
        update_prev_buffers(clear_bg_color);
        return_cursor();
        std::cout.flush();
        num_partial_redraws++;
      };

      switch (draw_policy)
      {
        case DrawPolicy::FULL:
          f_full_redraw(clear_bg_color);
          break;
        case DrawPolicy::PARTIAL:
          f_partial_redraw(clear_bg_color);
          break;
        case DrawPolicy::THRESHOLD_SELECT:
        {
          auto dirty_fraction = stlutils::count(dirty_flag_buffer, true) / (NR*NC);
          if (dirty_fraction > dirty_fraction_threshold)
            f_full_redraw(clear_bg_color);
          else
            f_partial_redraw(clear_bg_color);
          break;
        }
        case DrawPolicy::MEASURE_SELECT:
        {
          bool measured = false;
          if (frame % num_frames_between_measurings == 0)
          {
            if (measure_mode == 0)
            {
              benchmark::tic(t8_ScreenHandler_redraw_timer);
              f_full_redraw(clear_bg_color);
              measured_delay_ms_full = benchmark::toc(t8_ScreenHandler_redraw_timer);
            }
            else if (measure_mode == 1)
            {
              benchmark::tic(t8_ScreenHandler_redraw_timer);
              f_partial_redraw(clear_bg_color);
              measured_delay_ms_partial = benchmark::toc(t8_ScreenHandler_redraw_timer);
            }
            measure_mode = 1 - measure_mode;
            measured = true;
          }
          if (!measured)
          {
            if (measured_delay_ms_partial <= measured_delay_ms_full)
              f_partial_redraw(clear_bg_color);
            else
              f_full_redraw(clear_bg_color);
          }
          break;
        }
      }
      frame++;
    }
    
    void print_screen_buffer_full(Color clear_bg_color) const
    {
      Text::ComplexString<CharT> colored_str;
      colored_str.resize(NR*(NC + 1));
      int i = 0;
      for (int r = 0; r < NR; ++r)
      {
        for (int c = 0; c < NC; ++c)
        {
          int idx = index(r, c);
          Color bg_col_buf = bg_color_buffer[idx];
          if (bg_col_buf == Color16::Transparent || bg_col_buf == Color16::Transparent2)
            bg_col_buf = clear_bg_color;
          colored_str[i++] = { screen_buffer[idx], fg_color_buffer[idx], bg_col_buf };
        }
        colored_str[i++] = { static_cast<CharT>('\n'), Color16::Default, Color16::Default };
      }
      m_text->print_complex_sequential(colored_str);
    }
    
    void print_screen_buffer_partial(Color clear_bg_color) const
    {
      std::vector<Text::ComplexStringChunk<CharT>> colored_str_chunks;
      colored_str_chunks.reserve(math::roundI(num_chunks_prev * 1.2f));
      for (int r = 0; r < NR; ++r)
      {
        Text::ComplexStringChunk<CharT> chunk;
        chunk.text.reserve(16); // #FIXME: Magic number.
        for (int c = 0; c < NC; ++c)
        {
          int idx = index(r, c);
          if (dirty_flag_buffer[idx])
          {
            if (chunk.text.empty())
              chunk.pos = { r, c };
            Color bg_col_buf = bg_color_buffer[idx];
            if (bg_col_buf == Color16::Transparent || bg_col_buf == Color16::Transparent2)
              bg_col_buf = clear_bg_color;
            chunk.text.emplace_back(screen_buffer[idx], fg_color_buffer[idx], bg_col_buf);
          }
          else if (!chunk.text.empty())
          {
            colored_str_chunks.emplace_back(chunk);
            chunk.text.clear();
          }
        }
        // Flush.
        if (!chunk.text.empty())
          colored_str_chunks.emplace_back(std::move(chunk));
      }
      m_text->print_complex_chunks(colored_str_chunks);
      num_chunks_prev = stlutils::sizeI(colored_str_chunks);
    }
    
    void print_screen_buffer(Color bg_color, const OffscreenBuffer& offscreen_buffer)
    {
      auto* texture = offscreen_buffer.buffer_texture;
      if (texture == nullptr)
        return;
      
      auto pos = offscreen_buffer.buffer_screen_pos;
      
      for (int r = 0; r < NR; ++r)
      {
        auto rl = r - pos.r;
        for (int c = 0; c < NC; ++c)
        {
          auto cl = c - pos.c;
          
          auto textel = texture->operator()(rl, cl);
          if (!stlutils::contains(offscreen_buffer.dst_fill_bg_colors, textel.bg_color))
            continue;
          
          int idx = index(r, c);
          textel.glyph = screen_buffer[idx]; // #FIXME: Need to allow e.g. Glyph::str() => "[2603]". #glyph
          textel.fg_color = fg_color_buffer[idx];
          textel.bg_color = bg_color_buffer[idx];
          
          if (stlutils::contains(offscreen_buffer.exclude_src_chars, textel.glyph.preferred)) // #HACK! #glyph
            continue;
          if (stlutils::contains(offscreen_buffer.exclude_src_fg_colors, textel.fg_color))
            continue;
          if (stlutils::contains(offscreen_buffer.exclude_src_bg_colors, textel.bg_color))
            continue;
          
          for (const auto& rp : offscreen_buffer.replace_src_dst_bg_colors)
            if (textel.bg_color == rp.first)
              textel.bg_color = rp.second;
          
          texture->set_textel(rl, cl, textel);
        }
      }
    }
    
    std::vector<std::string> get_screen_buffer_chars() const
    {
      std::vector<std::string> ret(NR);
      for (int r = 0; r < NR; ++r)
      {
        auto& line = ret[r];
        line.resize(NC);
        for (int c = 0; c < NC; ++c)
          line[c] = screen_buffer[index(r, c)];
      }
      return ret;
    }
    
    Texture export_screen_buffers() const
    {
      Texture texture(NR, NC);
      for (int r = 0; r < NR; ++r)
      {
        for (int c = 0; c < NC; ++c)
        {
          int idx = index(r, c);
          texture.set_textel_char(r, c, screen_buffer[idx]);
          texture.set_textel_fg_color(r, c, fg_color_buffer[idx]);
          texture.set_textel_bg_color(r, c, bg_color_buffer[idx]);
        }
      }
      return texture;
    }
    
    void print_screen_buffer_chars() const
    {
      for (int r = 0; r < NR; ++r)
      {
        for (int c = 0; c < NC; ++c)
          printf("%c", screen_buffer[index(r, c)]);
        printf("\n");
      }
    }
    
    void print_screen_buffer_fg_colors() const
    {
      for (int r = 0; r < NR; ++r)
      {
        for (int c = 0; c < NC; ++c)
          printf("%s", color2str(fg_color_buffer[index(r, c)]).c_str());
        printf("\n");
      }
    }
    
    void print_screen_buffer_bg_colors() const
    {
      for (int r = 0; r < NR; ++r)
      {
        for (int c = 0; c < NC; ++c)
          printf("%s", color2str(bg_color_buffer[index(r, c)]).c_str());
        printf("\n");
      }
    }
    
    constexpr int num_rows() const { return NR; }
    constexpr int num_cols() const { return NC; }
    constexpr RC size() const { return { NR, NC }; }
    constexpr int num_rows_inset() const
    {
      auto nri = static_cast<int>(NR) - 2;
      return nri < 0 ? 0 : nri;
    }
    constexpr int num_cols_inset() const
    {
      auto nci = static_cast<int>(NC) - 2;
      return nci < 0 ? 0 : nci;
    }
    
    void overwrite_data(const std::array<CharT, NR*NC>& new_screen_buffer,
                        const std::array<Color, NR*NC>& new_fg_color_buffer,
                        const std::array<Color, NR*NC>& new_bg_color_buffer)
    {
      screen_buffer = new_screen_buffer;
      fg_color_buffer = new_fg_color_buffer;
      bg_color_buffer = new_bg_color_buffer;
    }
    
    template<int NRo, int NCo, int NRi, int NCi, typename char_t>
    friend void t8x::screen_scaling::resample(const ScreenHandler<NRi, NCi, char_t>& sh_src,
                                              ScreenHandler<NRo, NCo, char_t>& sh_dst);
  };
  
}
