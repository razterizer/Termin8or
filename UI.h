//
//  UI.h
//  Termin8or
//
//  Created by Rasmus Anthin on 2024-07-08.
//

#pragma once
#include <Core/StringBox.h>

namespace ui
{

  enum class VerticalAlignment { TOP, CENTER, BOTTOM };
  enum class HorizontalAlignment { LEFT, CENTER, RIGHT };

  class TextBox
  {
    str::StringBox sb;
    size_t N = 0;
    size_t len_max = 0;
    
    void init()
    {
      N = sb.size();
    }
    
  public:
    TextBox() = default;
    TextBox(const std::vector<std::string>& text_lines)
      : sb(text_lines)
    {
      init();
    }
    TextBox(const std::string& text)
      : sb(text)
    {
      init();
    }
    
    void set_text(const std::vector<std::string>& text_lines)
    {
      sb = str::StringBox { text_lines };
      init();
    }
    
    void set_text(const std::string& text)
    {
      sb = str::StringBox { text };
      init();
    }
    
    void calc_pre_draw(str::Adjustment adjustment)
    {
      len_max = 0;
      for (size_t l_idx = 0; l_idx < N; ++l_idx)
        math::maximize(len_max, sb[l_idx].size());
      for (size_t l_idx = 0; l_idx < N; ++l_idx)
        sb[l_idx] = str::adjust_str(sb[l_idx], adjustment, len_max);
    }
    
    template<int NR, int NC>
    void draw(SpriteHandler<NR, NC>& sh, const RC& pos, const styles::Style& box_style, bool draw_box_outline, bool draw_box_bkg, int box_padding_lr = 0, int box_padding_ud = 0, std::optional<styles::Style> box_outline_style = std::nullopt, drawing::OutlineType outline_type = drawing::OutlineType::Line)
    {
      for (size_t l_idx = 0; l_idx < N; ++l_idx)
        sh.write_buffer(sb[l_idx], pos.r + l_idx, pos.c, box_style);
      if (draw_box_outline)
        drawing::draw_box_outline(sh, pos.r - 1 - box_padding_ud, pos.c - 1 - box_padding_lr, N + 1 + 2*box_padding_ud, len_max + 1 + 2*box_padding_lr, outline_type, box_outline_style.value_or(box_style));
      if (draw_box_bkg)
        drawing::draw_box(sh, pos.r - 1 - box_padding_ud, pos.c - 1 - box_padding_lr, N + 1 + 2*box_padding_ud, len_max + 1 + 2*box_padding_lr, box_style);
    }
    
    template<int NR, int NC>
    void draw(SpriteHandler<NR, NC>& sh, VerticalAlignment v_align, HorizontalAlignment h_align, const styles::Style& style, bool draw_box_outline, bool draw_box_bkg, int box_padding_lr = 0, int box_padding_ud = 0, std::optional<styles::Style> box_outline_style = std::nullopt, drawing::OutlineType outline_type = drawing::OutlineType::Line)
    {
      RC pos { 0, 0 };
      
      switch (v_align)
      {
        case VerticalAlignment::TOP:
          pos.r = 0;
          break;
        case VerticalAlignment::CENTER:
          pos.r = std::round((NR - N)*0.5f);
          break;
        case VerticalAlignment::BOTTOM:
          pos.r = NR - N;
          break;
      }
      
      switch (h_align)
      {
        case HorizontalAlignment::LEFT:
          pos.c = 0;
          break;
        case HorizontalAlignment::CENTER:
          pos.c = std::round((NC - len_max)*0.5f);
          break;
        case HorizontalAlignment::RIGHT:
          pos.c = NC - len_max;
          break;
      }
      
      draw(sh, pos, style, draw_box_outline, draw_box_bkg, box_padding_lr, box_padding_ud, box_outline_style, outline_type);
    }
  };
  
  class TextBoxDebug : TextBox
  {
  
  };

}

