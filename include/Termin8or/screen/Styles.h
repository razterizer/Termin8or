//
//  Styles.h
//  Termin8or
//
//  Created by Rasmus Anthin on 2024-06-06.
//

#pragma once
#include "Color.h"

namespace t8
{

  struct Style
  {
    Style() = default;
    Style(Color16 fg, Color16 bg) : fg_color(fg), bg_color(bg) {}
    Color16 fg_color = Color16::Default;
    Color16 bg_color = Color16::Transparent;
    void swap() { std::swap(fg_color, bg_color); }
  };
  
  struct ButtonStyle : Style
  {
    ButtonStyle() = default;
    ButtonStyle(Color16 fg, Color16 bg, Color16 bg_sel)
      : Style(fg, bg), bg_color_selected(bg_sel) {}
    Color16 bg_color_selected = Color16::Transparent;
  };
  
  struct PromptStyle : Style
  {
    PromptStyle() = default;
    PromptStyle(Color16 fg, Color16 bg, Color16 bg_cursor)
      : Style(fg, bg), bg_color_cursor(bg_cursor) {}
    Color16 bg_color_cursor = Color16::Transparent;
  };
  
  struct HiliteFGStyle : Style
  {
    HiliteFGStyle() = default;
    HiliteFGStyle(Color16 fg, Color16 bg, Color16 fg_hilite)
      : Style(fg, bg), fg_color_hilite(fg_hilite) {}
    Color16 fg_color_hilite = Color16::Default;
    Style get_style(bool hilited) const
    {
      return { hilited ? fg_color_hilite : fg_color, bg_color };
    }
  };
  
  struct HiliteSelectFGStyle : HiliteFGStyle
  {
    HiliteSelectFGStyle(Color16 fg, Color16 bg, Color16 fg_hilite, Color16 fg_select, Color16 fg_select_hilite)
      : HiliteFGStyle(fg, bg, fg_hilite)
      , fg_color_select(fg_select)
      , fg_color_select_hilite(fg_select_hilite)
      {}
    Color16 fg_color_select = Color16::Default;
    Color16 fg_color_select_hilite = Color16::Default;
    Style get_style(bool hilited, bool selected) const
    {
      if (selected)
        return { hilited ? fg_color_select_hilite : fg_color_select, bg_color };
      return HiliteFGStyle::get_style(hilited);
    }
  };
  
  // ////
  
  Style make_shaded_style(Color16 base_color, ShadeType shade)
  {
    Style ret;
    switch (shade)
    {
      case ShadeType::Unchanged:
        ret.fg_color = base_color;
        ret.bg_color = base_color;
        break;
      case ShadeType::Bright:
        ret.fg_color = shade_color(base_color, ShadeType::Dark);
        ret.bg_color = shade_color(base_color, ShadeType::Bright);
        break;
      case ShadeType::Dark:
        ret.fg_color = shade_color(base_color, ShadeType::Bright);
        ret.bg_color = shade_color(base_color, ShadeType::Dark);
        break;
    }
    return ret;
  }
  
  Style shade_style(const Style& style, ShadeType shade, bool only_swap_if_fg_bg_same = false)
  {
    Style ret;
    switch (shade)
    {
      case ShadeType::Unchanged:
        ret = style;
        break;
      case ShadeType::Bright:
        ret.fg_color = shade_color(style.fg_color, ShadeType::Dark);
        ret.bg_color = shade_color(style.bg_color, ShadeType::Bright);
        break;
      case ShadeType::Dark:
        ret.fg_color = shade_color(style.fg_color, ShadeType::Bright);
        ret.bg_color = shade_color(style.bg_color, ShadeType::Dark);
        break;
    }
    if (only_swap_if_fg_bg_same && ret.fg_color == ret.bg_color)
    {
      ret = style;
      ret.swap();
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
