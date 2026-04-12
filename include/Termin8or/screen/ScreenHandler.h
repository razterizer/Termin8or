#pragma once
#include "Text.h"
#include "Styles.h"
#include "GlyphString.h"
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
#include <type_traits>
#include <variant>


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
  
  enum class AsciiFallbackPolicy { SYSTEM_CONTROLLED, FORCE_ASCII, FORCE_ASCII_ONLY_ON_WIN_CMD };
  
  // //////////////////////////////////////////////////
  
  template<int NR = 30, int NC = 80, typename CharT = char>
  class ScreenHandler
  {
    std::unique_ptr<Text> m_text;
    
    // Draw from top to bottom.
    std::array<BufferCell<CharT>, NC*NR> screen_buffer, prev_screen_buffer;
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
    mutable int num_chunks_prev = 10; // #FIXME: Magic number.
    
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
      const int c_tot = c + ci;
      if (c_tot < 0 || c_tot >= NC)
        return;
      
      int idx = index(r, c_tot);
      auto& [scr_ch, scr_fg, scr_bg] = screen_buffer[idx];
      
      auto set_glyph = [&]()
      {
        scr_ch = ch;
        scr_fg = fg_color;
      };
      
      if (scr_ch == static_cast<CharT>(' ')
          && scr_bg == Color16::Transparent)
      {
        set_glyph();
        scr_bg = bg_color;
      }
      else if (scr_bg == Color16::Transparent2)
      {
        scr_bg = bg_color;
        if (scr_ch == static_cast<CharT>(' '))
          set_glyph();
      }
    }
    
  public:
    ScreenHandler()
      : m_text(std::make_unique<Text>())
    {}
    
    void clear()
    {
      for (auto& cell : screen_buffer)
      {
        cell.ch = ' ';
        cell.fg = Color16::Default;
        cell.bg = Color16::Transparent;
      }
      for (auto& df : dirty_flag_buffer)
        df = false;
    }
    
    bool test_empty(int r, int c) const
    {
      return screen_buffer[index(r, c)].ch == static_cast<CharT>(' ');
    }
    
    void set_ascii_fallback_policy(AsciiFallbackPolicy policy)
    {
      switch (policy)
      {
        case AsciiFallbackPolicy::SYSTEM_CONTROLLED:
          term::force_ascii_fallback = false;
          break;
        case AsciiFallbackPolicy::FORCE_ASCII:
          term::force_ascii_fallback = true;
          break;
        case AsciiFallbackPolicy::FORCE_ASCII_ONLY_ON_WIN_CMD:
          term::force_ascii_fallback = term::m_term_mode.is_conhost_like;
          break;
      }
    }
    
    bool is_ascii_fallback_forced() const
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
    
    inline std::string encode_single_width_glyph(const Glyph& glyph) const
    {
      return t8::term::encode_single_width_glyph<CharT>(glyph.preferred, glyph.fallback);
    }
    
    // write_buffer using StringBox.
    template<typename StrT>
    void write_buffer(const str::StringBox<StrT>& sb, int r, int c, Color fg_color, Color bg_color = Color16::Transparent)
    {
      auto Nr = static_cast<int>(sb.text_lines.size());
      for (int r_idx = 0; r_idx < Nr; ++r_idx)
        write_buffer(sb.text_lines[r_idx], r + r_idx, c, fg_color, bg_color);
    }
    
    int write_buffer(const std::vector<StyledString>& ss_vec, int r, int c)
    {
      int h_pos = 0;
      for (const auto& ss : ss_vec)
      {
        write_buffer(ss.text, r, c + h_pos, ss.style);
        h_pos += ss.width;
      }
      return h_pos;
    }
    
    int write_buffer(const std::vector<StyledString>& ss_vec, const RC& pos)
    {
      return write_buffer(ss_vec, pos.r, pos.c);
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
      write_buffer(glyph, pos.r, pos.c, style.fg_color, style.bg_color);
    }
    
    void write_buffer(const Glyph& glyph, const RC& pos, Color fg_color, Color bg_color = Color16::Transparent)
    {
      write_buffer(glyph, pos.r, pos.c, fg_color, bg_color);
    }
    
    void write_buffer(const Glyph& glyph, int r, int c, const Style& style)
    {
      write_buffer(glyph, r, c, style.fg_color, style.bg_color);
    }
    
    void write_buffer(const Glyph& glyph, int r, int c, Color fg_color, Color bg_color = Color16::Transparent)
    {
      static_assert(std::is_same_v<CharT, char> || std::is_same_v<CharT, char32_t>,
                    "ERROR in ScreenHandler<NR, NC, CharT>::write_buffer() : Unsupported CharT type.");
    
      if (glyph.empty())
        return;
      if constexpr (std::is_same_v<CharT, char>)
      {
        if (r >= 0 && r < NR)
          write_buffer(encode_single_width_glyph(glyph), r, c, fg_color, bg_color);
      }
      else if constexpr (std::is_same_v<CharT, char32_t>)
      {
        if (r >= 0 && r < NR)
          write_buffer_cell(normalize_cp(term::resolve_single_width_glyph<CharT>(glyph.preferred, glyph.fallback)),
                            r, c, 0, fg_color, bg_color);
      }
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
      static_assert(std::is_same_v<CharT, char> || std::is_same_v<CharT, char32_t>,
                    "ERROR in ScreenHandler<NR, NC, CharT>::write_buffer() : Unsupported CharT type.");
      
      if (gstr.empty())
        return;
      if (r >= 0 && r < NR)
      {
        int n = static_cast<int>(gstr.size());
        for (int ci = 0; ci < n; ++ci)
          write_buffer(gstr[ci], r, c + ci, fg_color, bg_color);
      }
    }
    
    void write_buffer(char ch, const RC& pos, const Style& style)
    {
      write_buffer(ch, pos.r, pos.c, style.fg_color, style.bg_color);
    }
    
    void write_buffer(char ch, const RC& pos, Color fg_color, Color bg_color = Color16::Transparent)
    {
      write_buffer(ch, pos.r, pos.c, fg_color, bg_color);
    }
    
    void write_buffer(char ch, int r, int c, const Style& style)
    {
      write_buffer(ch, r, c, style.fg_color, style.bg_color);
    }
    
    void write_buffer(char ch, int r, int c, Color fg_color, Color bg_color = Color16::Transparent)
    {
      static_assert(std::is_same_v<CharT, char> || std::is_same_v<CharT, char32_t>,
                    "ERROR in ScreenHandler<NR, NC, CharT>::write_buffer() : Unsupported CharT type.");
      
      if (r >= 0 && r < NR)
        write_buffer_cell(static_cast<CharT>(normalize_byte(ch)), r, c, 0, fg_color, bg_color);
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
      static_assert(std::is_same_v<CharT, char> || std::is_same_v<CharT, char32_t>,
                    "ERROR in ScreenHandler<NR, NC, CharT>::write_buffer() : Unsupported CharT type.");
    
      if (str.empty())
        return;
      if constexpr (std::is_same_v<CharT, char>)
      {
        if (r >= 0 && r < NR)
        {
          int n = static_cast<int>(str.size());
          for (int ci = 0; ci < n; ++ci)
            write_buffer_cell(static_cast<char>(normalize_byte(str[ci])), r, c, ci, fg_color, bg_color);
        }
      }
      else if constexpr (std::is_same_v<CharT, char32_t>)
      {
        if (r >= 0 && r < NR)
        {
          int ci = 0;
          size_t byte_idx = 0;
          char32_t ch32 = utf8::none;
          while (utf8::decode_next_utf8_char32(str, ch32, byte_idx))
          {
            write_buffer_cell(term::get_renderable_char32(ch32), r, c, ci, fg_color, bg_color);
            ci++;
          }
        }
      }
    }
    
    void replace_bg_color(Color from_bg_color, Color to_bg_color, Rectangle box)
    {
      for (int r = 0; r < NR; ++r)
      {
        for (int c = 0; c < NC; ++c)
        {
          if (box.is_inside(r, c))
          {
            auto& col = screen_buffer[index(r, c)].bg;
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
            screen_buffer[index(r, c)].bg = to_bg_color;
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
          screen_buffer[index(r, c)].bg = to_bg_color;
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
            screen_buffer[index(r, c)].fg = to_fg_color;
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
          screen_buffer[index(r, c)].fg = to_fg_color;
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
          
          const auto& [ch_curr, fg_curr, bg0] = screen_buffer[idx];
          const auto& [ch_prev, fg_prev, bg1] = prev_screen_buffer[idx];
          
          auto bg_curr = resolve_bg_color(bg0, clear_bg_color);
          auto bg_prev = resolve_bg_color(bg1, prev_clear_bg_color);
          
          dirty_flag_buffer[idx] =
               ch_curr != ch_prev
            || fg_curr != fg_prev
            || bg_curr != bg_prev;
        }
      }
    }
    
    void update_prev_buffers(Color clear_bg_color)
    {
      prev_screen_buffer = screen_buffer;
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
    
    void print_screen_buffer(Color clear_bg_color, Color empty_fg_color = Color16::Default, DrawPolicy draw_policy = DrawPolicy::MEASURE_SELECT)
    {
      auto f_full_redraw = [this](Color clear_bg_color, Color empty_fg_color)
      {
        print_screen_buffer_full(clear_bg_color, empty_fg_color);
        update_prev_buffers(clear_bg_color); // Otherwise prev_screen_buffer will go stale and next partial draw will treat many cells as dirty.
        num_full_redraws++;
      };
    
      auto f_partial_redraw = [this](Color clear_bg_color, Color empty_fg_color)
      {
        diff_buffers(clear_bg_color);
        print_screen_buffer_partial(clear_bg_color, empty_fg_color);
        update_prev_buffers(clear_bg_color);
        return_cursor();
        std::cout.flush();
        num_partial_redraws++;
      };

      switch (draw_policy)
      {
        case DrawPolicy::FULL:
          f_full_redraw(clear_bg_color, empty_fg_color);
          break;
        case DrawPolicy::PARTIAL:
          f_partial_redraw(clear_bg_color, empty_fg_color);
          break;
        case DrawPolicy::THRESHOLD_SELECT:
        {
          auto dirty_fraction = stlutils::count(dirty_flag_buffer, true) / static_cast<float>(NR*NC);
          if (dirty_fraction > dirty_fraction_threshold)
            f_full_redraw(clear_bg_color, empty_fg_color);
          else
            f_partial_redraw(clear_bg_color, empty_fg_color);
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
              f_full_redraw(clear_bg_color, empty_fg_color);
              measured_delay_ms_full = benchmark::toc(t8_ScreenHandler_redraw_timer);
            }
            else if (measure_mode == 1)
            {
              benchmark::tic(t8_ScreenHandler_redraw_timer);
              f_partial_redraw(clear_bg_color, empty_fg_color);
              measured_delay_ms_partial = benchmark::toc(t8_ScreenHandler_redraw_timer);
            }
            measure_mode = 1 - measure_mode;
            measured = true;
          }
          if (!measured)
          {
            if (measured_delay_ms_partial <= measured_delay_ms_full)
              f_partial_redraw(clear_bg_color, empty_fg_color);
            else
              f_full_redraw(clear_bg_color, empty_fg_color);
          }
          break;
        }
      }
      frame++;
    }
    
    void print_screen_buffer_full(Color clear_bg_color, Color empty_fg_color) const
    {
      Text::OutputStringSeq<CharT> colored_str;
      colored_str.resize(NR*(NC + 1));
      int i = 0;
      for (int r = 0; r < NR; ++r)
      {
        for (int c = 0; c < NC; ++c)
        {
          int idx = index(r, c);
          const auto& cell = screen_buffer[idx];
          auto fg_col = cell.fg;
          if (fg_col == Color16::Transparent || fg_col == Color16::Transparent2)
            fg_col = empty_fg_color;
          auto bg_col = cell.bg;
          if (bg_col == Color16::Transparent || bg_col == Color16::Transparent2)
            bg_col = clear_bg_color;
          colored_str[i++] = { cell.ch, fg_col, bg_col };
        }
        colored_str[i++] = { static_cast<CharT>('\n'), Color16::Default, Color16::Default };
      }
      m_text->emit_sequential(colored_str);
    }
    
    void print_screen_buffer_partial(Color clear_bg_color, Color empty_fg_color) const
    {
      std::vector<Text::OutputStringChunk<CharT>> colored_str_chunks;
      colored_str_chunks.reserve(math::roundI(num_chunks_prev * 1.2f));
      for (int r = 0; r < NR; ++r)
      {
        Text::OutputStringChunk<CharT> chunk;
        chunk.text.reserve(16); // #FIXME: Magic number.
        for (int c = 0; c < NC; ++c)
        {
          int idx = index(r, c);
          if (dirty_flag_buffer[idx])
          {
            if (chunk.text.empty())
              chunk.pos = { r, c };
            const auto& cell = screen_buffer[idx];
            auto fg_col = cell.fg;
            if (fg_col == Color16::Transparent || fg_col == Color16::Transparent2)
              fg_col = empty_fg_color;
            auto bg_col = cell.bg;
            if (bg_col == Color16::Transparent || bg_col == Color16::Transparent2)
              bg_col = clear_bg_color;
            chunk.text.emplace_back(cell.ch, fg_col, bg_col);
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
      m_text->emit_chunks(colored_str_chunks);
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
        if (!texture->check_range_r(rl))
          continue;
        for (int c = 0; c < NC; ++c)
        {
          auto cl = c - pos.c;
          if (!texture->check_range_c(cl))
            continue;
          
          auto textel = texture->operator()(rl, cl);
          if (!stlutils::contains(offscreen_buffer.dst_fill_bg_colors, textel.bg_color))
            continue;
          
          int idx = index(r, c);
          const auto& [ch, fg, bg] = screen_buffer[idx];
          textel.glyph = ch; // #FIXME: Need to allow e.g. Glyph::str() => "[2603]". #glyph
          textel.fg_color = fg;
          textel.bg_color = bg;
          
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
      static_assert(std::is_same_v<CharT, char> || std::is_same_v<CharT, char32_t>,
                    "ERROR in ScreenHandler<NR, NC, CharT>::get_screen_buffer_chars() : Unsupported CharT type.");
    
      std::vector<std::string> ret(NR);
      for (int r = 0; r < NR; ++r)
      {
        auto& line = ret[r];
        if constexpr (std::is_same_v<CharT, char>)
        {
          line.resize(NC);
          for (int c = 0; c < NC; ++c)
            line[c] = screen_buffer[index(r, c)].ch;
        }
        else if constexpr (std::is_same_v<CharT, char32_t>)
        {
          line.reserve(static_cast<size_t>(NC) * 4); // Worst case.
          for (int c = 0; c < NC; ++c)
            line += encode_single_width_glyph(screen_buffer[index(r, c)].ch);
        }
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
          const auto& [ch, fg, bg] = screen_buffer[idx];
          texture.set_textel_fg_color(r, c, fg);
          texture.set_textel_bg_color(r, c, bg);
          if constexpr (std::is_same_v<CharT, char>)
            texture.set_textel_char(r, c, ch);
          else if constexpr (std::is_same_v<CharT, char32_t>)
          {
            if (term::is_printable_ascii(ch))
              texture.set_textel_char(r, c, static_cast<char>(ch));
            else
              texture.set_textel_glyph(r, c, { ch, '?' });
          }
        }
      }
      return texture;
    }
    
    void print_screen_buffer_chars() const
    {
      auto line_vec = get_screen_buffer_chars();
      for (const auto& line : line_vec)
        std::cout << line << '\n';
    }
    
    void print_screen_buffer_fg_colors() const
    {
      for (int r = 0; r < NR; ++r)
      {
        for (int c = 0; c < NC; ++c)
          printf("%s", color2str(screen_buffer[index(r, c)].fg).c_str());
        printf("\n");
      }
    }
    
    void print_screen_buffer_bg_colors() const
    {
      for (int r = 0; r < NR; ++r)
      {
        for (int c = 0; c < NC; ++c)
          printf("%s", color2str(screen_buffer[index(r, c)].bg).c_str());
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
    
    void overwrite_data(const std::array<BufferCell<CharT>, NR*NC>& new_screen_buffer)
    {
      screen_buffer = new_screen_buffer;
    }
    
    template<int NRo, int NCo, int NRi, int NCi, typename char_t>
    friend void t8x::screen_scaling::resample(const ScreenHandler<NRi, NCi, char_t>& sh_src,
                                              ScreenHandler<NRo, NCo, char_t>& sh_dst);
  };
  
}
