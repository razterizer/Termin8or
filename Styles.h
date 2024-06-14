//
//  Styles.h
//  Termin8or
//
//  Created by Rasmus Anthin on 2024-06-06.
//

#pragma once
#include "Text.h"

namespace styles
{

  struct Style
  {
    Style() = default;
    Style(Text::Color fg, Text::Color bg) : fg_color(fg), bg_color(bg) {}
    Text::Color fg_color = Text::Color::Default;
    Text::Color bg_color = Text::Color::Transparent;
  };
  
  struct ButtonStyle : Style
  {
    ButtonStyle() = default;
    ButtonStyle(Text::Color fg, Text::Color bg, Text::Color bg_sel)
      : Style(fg, bg), bg_color_selected(bg_sel) {}
    Text::Color bg_color_selected = Text::Color::Transparent;
  };
  
  struct PromptStyle : Style
  {
    PromptStyle() = default;
    PromptStyle(Text::Color fg, Text::Color bg, Text::Color bg_cursor)
      : Style(fg, bg), bg_color_cursor(bg_cursor) {}
    Text::Color bg_color_cursor = Text::Color::Transparent;
  };
  
  struct HiliteFGStyle : Style
  {
    HiliteFGStyle() = default;
    HiliteFGStyle(Text::Color fg, Text::Color bg, Text::Color fg_hilite)
      : Style(fg, bg), fg_color_hilite(fg_hilite) {}
    Text::Color fg_color_hilite = Text::Color::Default;
  };
  
  Style get_random_style(const std::vector<Style>& palette)
  {
    auto num = static_cast<int>(palette.size());
    if (num == 0)
      return Style {};
    auto idx = rnd::rand_int(0, num - 1);
    return palette[idx];
  }
  
  
}
