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
  };
  
  class TextBoxCentered : TextBox
  {
  
  };
  
  class TextBoxDebug : TextBox
  {
  
  };

}

