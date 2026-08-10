//
//  TextBox.h
//  Termin8or
//
//  Created by Rasmus Anthin on 2026-05-10.
//

#pragma once
#include "Widget.h"
#include "../../geom/RC.h"
#include "../../screen/Styles.h"
#include "../../screen/ScreenHandler.h"
#include "../../drawing/Drawing.h" // OutlineType
#include <Core/StringHelper.h>
#include <string>
#include <optional>
#include <vector>
#include <utility> // std::pair

namespace t8x
{

  using RC = t8::RC;
  using Style = t8::Style;
  template<int NR, int NC, typename CharT>
  using ScreenHandler = t8::ScreenHandler<NR, NC, CharT>;

  enum class VerticalAlignment { TOP, CENTER, BOTTOM };
  enum class HorizontalAlignment { LEFT, CENTER, RIGHT };
  
  struct TextBoxDrawingArgs
  {
    Style box_style;
    bool draw_box_outline = true;
    bool draw_box_bkg = true;
    int box_padding_ud = 0;
    int box_padding_lr = 0;
    std::optional<Style> box_outline_style = std::nullopt;
    OutlineType outline_type = OutlineType::Line;
  };
  
  struct TextBoxDrawingArgsPos
  {
    TextBoxDrawingArgs base;
    RC pos;
  };
  
  struct TextBoxDrawingArgsAlign
  {
    TextBoxDrawingArgs base;
    VerticalAlignment v_align = VerticalAlignment::CENTER;
    HorizontalAlignment h_align = HorizontalAlignment::CENTER;
    int v_align_offs = 0;
    int h_align_offs = 0;
    bool framed_mode = true;
  };
  
  // ////
  
  // /////////////////////////////////////////////////////////////
  //  _______        _   ____                                   //
  // |__   __|      | | |  _ \                                  //
  //    | | _____  _| |_| |_) | _____  __                       //
  //    | |/ _ \ \/ / __|  _ < / _ \ \/ /                       //
  //    | |  __/>  <| |_| |_) | (_) >  <                        //
  //    |_|\___/_/\_\\__|____/ \___/_/\_\                       //
  // /////////////////////////////////////////////////////////////

  // #NOTE: Supported template argument types: std::string and t8::GlyphString.
  template<typename StrT = std::string>
  class TextBox
  {
    bool allow_panel_size_caching = false;
  
  protected:
    str::StringBox<StrT> sb;
    size_t N = 0;
    size_t len_max = 0;
    std::vector<Style> line_styles;
    std::vector<std::pair<RC, Style>> override_textel_styles;
    std::optional<RC> cached_panel_size = std::nullopt;
    str::Adjustment master_adjustment = str::Adjustment::Left;
    
    void init()
    {
      N = sb.size();
      invalidate_layout();
    }
    
    virtual bool has_buttons() const { return false; }
    
    template<int NR, int NC>
    TextBoxDrawingArgsPos get_drawing_args_pos(const TextBoxDrawingArgsAlign& args)
    {
      int box_padding_ud = args.base.box_padding_ud;
      int box_padding_lr = args.base.box_padding_lr;
      
      const auto panel_size = fetch_panel_size();
      int panel_height = panel_size.r;
      int panel_width = panel_size.c;
    
      RC pos { 0, 0 };
      
      auto r_diff = std::max(0, NR - panel_height);
      auto c_diff = std::max(0, NC - panel_width);
      auto mid_v = static_cast<int>(std::round(r_diff*0.5f) - (r_diff%2 == 1)*0.5f);
      auto mid_h = static_cast<int>(std::round(c_diff*0.5f) - (c_diff%2 == 1)*0.5f);
      
      switch (args.v_align)
      {
        case VerticalAlignment::TOP:
          pos.r = 2 + box_padding_ud - !args.framed_mode;
          break;
        case VerticalAlignment::CENTER:
          pos.r = mid_v;
          break;
        case VerticalAlignment::BOTTOM:
          pos.r = static_cast<int>(NR - N) - box_padding_ud - 2 + !args.framed_mode; // FIXED!
          break;
      }
      
      switch (args.h_align)
      {
        case HorizontalAlignment::LEFT:
          pos.c = 2 + box_padding_lr - !args.framed_mode;
          break;
        case HorizontalAlignment::CENTER:
          pos.c = mid_h;
          break;
        case HorizontalAlignment::RIGHT:
          pos.c = static_cast<int>(NC - len_max) - box_padding_lr - 2 + !args.framed_mode; // FIXED!
          break;
      }
      
      pos.r += args.v_align_offs;
      pos.c += args.h_align_offs;
      
      TextBoxDrawingArgsPos pargs;
      pargs.pos = pos;
      pargs.base = args.base;
      
      return pargs;
    }
    
    virtual RC measure_panel_size() const
    {
      int r_len = static_cast<int>(TextBox<StrT>::N);
      int c_len = static_cast<int>(TextBox<StrT>::len_max);
      return { r_len, c_len };
    }
    
    RC fetch_panel_size()
    {
      if (!cached_panel_size.has_value() || !allow_panel_size_caching)
      {
        N = sb.size();
        len_max = 0;
        for (size_t l_idx = 0; l_idx < N; ++l_idx)
          math::maximize(len_max, sb[l_idx].size());
        
        cached_panel_size = measure_panel_size();
        
        for (size_t l_idx = 0; l_idx < N; ++l_idx)
          sb[l_idx] = str::adjust_str(sb[l_idx], master_adjustment, static_cast<int>(len_max));
      }
      return cached_panel_size.value();
    }
    
    virtual void on_pre_draw() {}
    
  public:
    virtual ~TextBox() = default;
    TextBox() = default;
    TextBox(str::Adjustment master_adj, bool cache_panel_size)
      : allow_panel_size_caching(cache_panel_size)
      , master_adjustment(master_adj)
    {}
    TextBox(size_t num_lines, str::Adjustment master_adj = str::Adjustment::Left)
      : sb(num_lines)
      , master_adjustment(master_adj)
    {
      init();
    }
    TextBox(const std::vector<StrT>& text_lines, const std::vector<Style>& styles = {},
            str::Adjustment master_adj = str::Adjustment::Left)
      : sb(text_lines)
      , line_styles(styles)
      , master_adjustment(master_adj)
    {
      init();
      if (text_lines.size() != line_styles.size())
        line_styles.clear();
    }
    TextBox(const StrT& text, str::Adjustment master_adj = str::Adjustment::Left)
      : sb(text)
      , master_adjustment(master_adj)
    {
      init();
    }
        
    bool empty() const noexcept
    {
      return sb.empty();
    }
    
    void set_text(const std::vector<StrT>& text_lines,
                  const std::vector<Style>& styles = {},
                  const std::vector<std::pair<RC, Style>>& override_styles = {},
                  str::Adjustment master_adj = str::Adjustment::Left)
    {
      sb = str::StringBox<StrT> { text_lines };
      line_styles = styles;
      override_textel_styles = override_styles;
      master_adjustment = master_adj;
      init();
      if (text_lines.size() != line_styles.size())
        line_styles.clear();
    }
    
    void set_text(const StrT& text,
                  str::Adjustment master_adj = str::Adjustment::Left)
    {
      sb = str::StringBox<StrT> { text };
      override_textel_styles.clear();
      master_adjustment = master_adj;
      init();
    }
    
    void set_text(const StrT& text, Style style, str::Adjustment master_adj = str::Adjustment::Left)
    {
      sb = str::StringBox<StrT> { text };
      line_styles.clear();
      line_styles.emplace_back(style);
      override_textel_styles.clear();
      master_adjustment = master_adj;
      init();
    }
    
    void set_num_lines(size_t num_lines)
    {
      sb = str::StringBox<StrT>(num_lines);
      init();
    }
    
    StrT& operator[](size_t r_idx)
    {
      return sb[r_idx];
      //invalidate_layout(); // We would want this to be called automatically after the reference has been used.
    }
    const StrT& operator[](size_t r_idx) const
    {
      return sb[r_idx];
    }
    
    template<int NR, int NC, typename CharT>
    void draw(ScreenHandler<NR, NC, CharT>& sh, const TextBoxDrawingArgsPos& args)
    {
      on_pre_draw();
    
      const auto& pos = args.pos;
      auto panel_size = fetch_panel_size();
      int panel_height = panel_size.r;
      int panel_width = panel_size.c;
      const Style& box_style = args.base.box_style;
      bool do_draw_box_outline = args.base.draw_box_outline;
      bool do_draw_box_bkg = args.base.draw_box_bkg;
      int box_padding_ud = args.base.box_padding_ud;
      int box_padding_lr = args.base.box_padding_lr;
#ifdef __linux__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
#endif
      std::optional<Style> box_outline_style = args.base.box_outline_style;
#ifdef __linux__
#pragma GCC diagnostic pop
#endif
      OutlineType outline_type = args.base.outline_type;
      
      for (const auto& rc_style : override_textel_styles)
        if (rc_style.first.r < static_cast<int>(N) && rc_style.first.c < static_cast<int>(len_max))
          sh.write_buffer(sb[rc_style.first.r][rc_style.first.c],
                          pos.r + rc_style.first.r, pos.c + rc_style.first.c,
                          rc_style.second);
      for (size_t l_idx = 0; l_idx < N; ++l_idx)
        sh.write_buffer(sb[l_idx], pos.r + static_cast<int>(l_idx), pos.c, line_styles.empty() ? box_style : line_styles[l_idx]);
      
      int r = pos.r - 1 - box_padding_ud;
      int c = pos.c - 1 - box_padding_lr;
      int r_len = panel_height + static_cast<int>(2 + 2*has_buttons() + 2*box_padding_ud);
      int c_len = panel_width + static_cast<int>(2 + 2*box_padding_lr);
      if (do_draw_box_outline)
        draw_box_outline(sh, r, c, r_len, c_len, outline_type, box_outline_style.value_or(box_style));
      if (do_draw_box_bkg)
        draw_box(sh, r, c, r_len, c_len, box_style);
    }
    
    template<int NR, int NC, typename CharT>
    void draw(ScreenHandler<NR, NC, CharT>& sh, const TextBoxDrawingArgsAlign& args)
    {
      auto pargs = get_drawing_args_pos<NR, NC>(args);
      
      draw(sh, pargs);
    }
    
    void invalidate_layout()
    {
      cached_panel_size = std::nullopt;
    }
  };
  

}
