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
    StringBox sb;
    size_t N = 0;
    size_t len_max = 0;
    
    void init()
    {
      N = sb.size();
      for (size_t l_idx = 0; l_idx < N; ++l_idx)
        math::maximize(len_max, sb[l_idx]);
    }
    
  public:
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
      for (size_t l_idx = 0; l_idx < N; ++l_idx)
        sb[l_idx] = str::adjust_str(sb[l_idx], adjustment, len_max);
    }
    
    template<int NR, int NC>
    void draw(SpriteHandler<NR, NC>& sh, const RC& pos, const styles::Style& style, bool draw_outline, bool draw_bkg, int box_padding = 0)
    {
      for (size_t l_idx = 0; l_idx < N; ++l_idx)
        sh.write_buffer(sb[l_idx], pos.r, pos.c, style);
    }
  };
  
  class TextBoxCentered : TextBox
  {
  
  };
  
  class TextBoxDebug : TextBox
  {
  
  };

}

