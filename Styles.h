//
//  Styles.h
//  Termin8or
//
//  Created by Rasmus Anthin on 2024-06-06.
//

#pragma once
#include "Color.h"

namespace styles
{

  struct Style
  {
    Style() = default;
    Style(Color fg, Color bg) : fg_color(fg), bg_color(bg) {}
    Color fg_color = Color::Default;
    Color bg_color = Color::Transparent;
    void swap() { std::swap(fg_color, bg_color); }
  };
  
  struct ButtonStyle : Style
  {
    ButtonStyle() = default;
    ButtonStyle(Color fg, Color bg, Color bg_sel)
      : Style(fg, bg), bg_color_selected(bg_sel) {}
    Color bg_color_selected = Color::Transparent;
  };
  
  struct PromptStyle : Style
  {
    PromptStyle() = default;
    PromptStyle(Color fg, Color bg, Color bg_cursor)
      : Style(fg, bg), bg_color_cursor(bg_cursor) {}
    Color bg_color_cursor = Color::Transparent;
  };
  
  struct HiliteFGStyle : Style
  {
    HiliteFGStyle() = default;
    HiliteFGStyle(Color fg, Color bg, Color fg_hilite)
      : Style(fg, bg), fg_color_hilite(fg_hilite) {}
    Color fg_color_hilite = Color::Default;
    Style get_style(bool selected) const
    {
      return { selected ? fg_color_hilite : fg_color, bg_color };
    }
  };
  
  // ////
  
  Style make_shaded_style(Color base_color, color::ShadeType shade)
  {
    Style ret;
    switch (shade)
    {
      case color::ShadeType::Bright:
        ret.fg_color = color::shade_color(base_color, color::ShadeType::Dark);
        ret.bg_color = color::shade_color(base_color, color::ShadeType::Bright);
        break;
      case color::ShadeType::Dark:
        ret.fg_color = color::shade_color(base_color, color::ShadeType::Bright);
        ret.bg_color = color::shade_color(base_color, color::ShadeType::Dark);
        break;
    }
    return ret;
  }
  
  Style shade_style(const Style& style, color::ShadeType shade)
  {
    Style ret;
    switch (shade)
    {
      case color::ShadeType::Bright:
        ret.fg_color = color::shade_color(style.fg_color, color::ShadeType::Dark);
        ret.bg_color = color::shade_color(style.bg_color, color::ShadeType::Bright);
        break;
      case color::ShadeType::Dark:
        ret.fg_color = color::shade_color(style.fg_color, color::ShadeType::Bright);
        ret.bg_color = color::shade_color(style.bg_color, color::ShadeType::Dark);
        break;
    }
    return ret;
  }
  
  Style get_random_style(const std::vector<Style>& palette)
  {
    auto num = static_cast<int>(palette.size());
    if (num == 0)
      return Style {};
    auto idx = rnd::rand_int(0, num - 1);
    return palette[idx];
  }
  
  
}
