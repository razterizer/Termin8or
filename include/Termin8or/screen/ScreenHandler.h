#pragma once
#include "Text.h"
#include "Styles.h"
#include "../geom/Rectangle.h"
#include "../geom/RC.h"
#include "../drawing/Texture.h"
#include <Core/StringHelper.h>
#include <Core/StringBox.h>
#include <Core/StlUtils.h>
#include <Core/Math.h>
#include <Core/Benchmark.h>
#include <Core/System.h>
#include <array>
#include <map>
#include <memory>


namespace t8
{
  
  template<int NR, int NC>
  class ScreenHandler;
  
  namespace screen_scaling
  {
    template<int NRo, int NCo, int NRi, int NCi>
    void resample(const ScreenHandler<NRi, NCi>& sh_src,
                  ScreenHandler<NRo, NCo>& sh_dst);
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
    std::vector<char> exclude_src_chars;
    std::vector<Color> exclude_src_fg_colors;
    std::vector<Color> exclude_src_bg_colors;
    std::vector<Color> dst_fill_bg_colors;
    std::vector<std::pair<Color, Color>> replace_src_dst_bg_colors;
  };
  
  enum class DrawPolicy { FULL, SUGGEST_PARTIAL, FORCE_PARTIAL, THRESHOLD_SELECT, MEASURE_SELECT };
  
  // //////////////////////////////////////////////////
  
  // Bad design.
  static int num_chunks_prev = 10; // #FIXME: Magic number.
  
  template<int NR = 30, int NC = 80>
  class ScreenHandler
  {
    std::unique_ptr<Text> m_text;
    
    // Draw from top to bottom.
    std::array<char, NC*NR> screen_buffer, prev_screen_buffer;
    std::array<Color, NC*NR> fg_color_buffer, prev_fg_color_buffer;
    std::array<Color, NC*NR> bg_color_buffer, prev_bg_color_buffer;
    std::array<bool, NC*NR> dirty_flag_buffer;
    Color prev_clear_bg_color = Color::Default;
    
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
      switch (col)
      {
        case Color::Transparent:  return "---:";
        case Color::Transparent2: return "===:";
        case Color::Default:      return "Def:";
        case Color::Black:        return "Blk:";
        case Color::DarkRed:      return "DRd:";
        case Color::DarkGreen:    return "DGn:";
        case Color::DarkYellow:   return "DYw:";
        case Color::DarkBlue:     return "DBu:";
        case Color::DarkMagenta:  return "DMa:";
        case Color::DarkCyan:     return "DCn:";
        case Color::LightGray:    return "LGy:";
        case Color::DarkGray:     return "DGy:";
        case Color::Red:          return "Red:";
        case Color::Green:        return "Grn:";
        case Color::Yellow:       return "Ylw:";
        case Color::Blue:         return "Blu:";
        case Color::Magenta:      return "Mga:";
        case Color::Cyan:         return "Cyn:";
        case Color::White:        return "Wht:";
        default: break;
      }
      return "";
    }
    
    std::vector<OrderedText> ordered_texts;
    
  public:
    ScreenHandler()
      : m_text(std::make_unique<Text>())
    {}
    
    void clear()
    {
      for (auto& ch : screen_buffer)
        ch = ' ';
      for (auto& col : fg_color_buffer)
        col = Color::Default;
      for (auto& col : bg_color_buffer)
        col = Color::Transparent;
      for (auto& df : dirty_flag_buffer)
        df = false;
    }
    
    bool test_empty(int r, int c) const
    {
      return screen_buffer[index(r, c)] == ' ';
    }
    
    // write_buffer using StringBox.
    void write_buffer(const str::StringBox& sb, int r, int c, Color fg_color, Color bg_color = Color::Transparent)
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
    
    void write_buffer(const std::string& str, const RC& pos, const Style& style)
    {
      write_buffer(str, pos.r, pos.c, style.fg_color, style.bg_color);
    }
    
    void write_buffer(const std::string& str, const RC& pos, Color fg_color, Color bg_color = Color::Transparent)
    {
      write_buffer(str, pos.r, pos.c, fg_color, bg_color);
    }
    
    void write_buffer(const std::string& str, int r, int c, const Style& style)
    {
      write_buffer(str, r, c, style.fg_color, style.bg_color);
    }
    
    void write_buffer(const std::string& str, int r, int c, Color fg_color, Color bg_color = Color::Transparent)
    {
      if (str.empty())
        return;
      //if (c >= 0 && str.size() + c <= NC)
      {
        if (r >= 0 && r < NR)
        {
          int n = static_cast<int>(str.size());
          for (int ci = 0; ci < n; ++ci)
          {
            int c_tot = c + ci;
            if (c_tot >= 0 && c_tot < NC)
            {
              int idx = index(r, c_tot);
              auto& scr_ch = screen_buffer[idx];
              auto& scr_fg = fg_color_buffer[idx];
              auto& scr_bg = bg_color_buffer[idx];
              if (scr_ch == ' '
                && scr_bg == Color::Transparent)
              {
                scr_ch = str[ci];
                scr_fg = fg_color;
                scr_bg = bg_color;
              }
              else if (scr_bg == Color::Transparent2)
              {
                scr_bg = bg_color;
                if (scr_ch == ' ')
                {
                  scr_ch = str[ci];
                  scr_fg = fg_color;
                }
              }
            }
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
      return (bg_color == Color::Transparent || bg_color == Color::Transparent2) ? clear_bg_color : bg_color;
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
    
    void print_screen_buffer(Color clear_bg_color, DrawPolicy posix_draw_policy = DrawPolicy::MEASURE_SELECT)
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

      if (sys::is_windows() || sys::is_wsl())
      {
        // Partial redraw doesn't seem to work so well on Windows
        //   so therefore we're always doing a full redraw.
        f_full_redraw(clear_bg_color);
      }
      else
      {
        switch (posix_draw_policy)
        {
          case DrawPolicy::FULL:
            f_full_redraw(clear_bg_color);
            break;
          case DrawPolicy::SUGGEST_PARTIAL:
          case DrawPolicy::FORCE_PARTIAL:
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
      }
      frame++;
    }
    
    void print_screen_buffer_full(Color clear_bg_color) const
    {
      std::vector<std::tuple<char, Color, Color>> colored_str;
      colored_str.resize(NR*(NC + 1));
      int i = 0;
      for (int r = 0; r < NR; ++r)
      {
        for (int c = 0; c < NC; ++c)
        {
          int idx = index(r, c);
          Color bg_col_buf = bg_color_buffer[idx];
          if (bg_col_buf == Color::Transparent || bg_col_buf == Color::Transparent2)
            bg_col_buf = clear_bg_color;
          colored_str[i++] = { screen_buffer[idx], fg_color_buffer[idx], bg_col_buf };
        }
        colored_str[i++] = { '\n', Color::Default, Color::Default };
      }
      m_text->print_complex_sequential(colored_str);
    }
    
    void print_screen_buffer_partial(Color clear_bg_color) const
    {
      std::vector<Text::ComplexStringChunk> colored_str_chunks;
      colored_str_chunks.reserve(math::roundI(num_chunks_prev * 1.2f));
      for (int r = 0; r < NR; ++r)
      {
        Text::ComplexStringChunk chunk;
        chunk.text.reserve(16); // #FIXME: Magic number.
        for (int c = 0; c < NC; ++c)
        {
          int idx = index(r, c);
          if (dirty_flag_buffer[idx])
          {
            if (chunk.text.empty())
              chunk.pos = { r, c };
            Color bg_col_buf = bg_color_buffer[idx];
            if (bg_col_buf == Color::Transparent || bg_col_buf == Color::Transparent2)
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
          textel.ch = screen_buffer[idx];
          textel.fg_color = fg_color_buffer[idx];
          textel.bg_color = bg_color_buffer[idx];
          
          if (stlutils::contains(offscreen_buffer.exclude_src_chars, textel.ch))
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
    
    void overwrite_data(const std::array<std::array<char, NC>, NR>& new_screen_buffer,
                        const std::array<std::array<Color, NC>, NR>& new_fg_color_buffer,
                        const std::array<std::array<Color, NC>, NR>& new_bg_color_buffer)
    {
      screen_buffer = new_screen_buffer;
      fg_color_buffer = new_fg_color_buffer;
      bg_color_buffer = new_bg_color_buffer;
    }
    
    template<int NRo, int NCo, int NRi, int NCi>
    friend void screen_scaling::resample(const ScreenHandler<NRi, NCi>& sh_src,
                                         ScreenHandler<NRo, NCo>& sh_dst);
  };
  
}
