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

  struct WidgetStyle
  {
    WidgetStyle(Text::Color fg, Text::Color bg) : fg_color(fg), bg_color(bg) {}
    Text::Color fg_color = Text::Color::Default;
    Text::Color bg_color = Text::Color::Transparent;
  };
  
  struct ButtonStyle : WidgetStyle
  {
    ButtonStyle(Text::Color fg, Text::Color bg, Text::Color bg_sel)
      : WidgetStyle(fg, bg), bg_color_selected(bg_sel) {}
    Text::Color bg_color_selected = Text::Color::Transparent;
  };
  
  struct PromptStyle : WidgetStyle
  {
    PromptStyle(Text::Color fg, Text::Color bg, Text::Color bg_cursor)
      : WidgetStyle(fg, bg), bg_color_cursor(bg_cursor) {}
    Text::Color bg_color_cursor = Text::Color::Transparent;
  };
  
}
