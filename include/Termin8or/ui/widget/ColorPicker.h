//
//  ColorPicker.h
//  Termin8or
//
//  Created by Rasmus Anthin on 2026-05-10.
//

#pragma once
#include "Widget.h"
#include "../../geom/RC.h"
#include "../../input/KeyboardEnums.h"
#include "../../screen/Color.h"
#include "../../screen/ScreenHandler.h"

namespace t8x
{

  using RC = t8::RC;
  using SpecialKey = t8::SpecialKey;
  using Color = t8::Color;
  template<int NR, int NC, typename CharT>
  using ScreenHandler = t8::ScreenHandler<NR, NC, CharT>;

  enum class ColorPickerCursorColoring { BlackWhite, Contrast };
  enum class ColorPickerCursorBlinking { Persist, EveryOther };
  
  struct ColorPickerParams
  {
    ColorPickerCursorColoring fg_cursor_coloring = ColorPickerCursorColoring::BlackWhite;
    ColorPickerCursorBlinking cursor_blinking = ColorPickerCursorBlinking::Persist;
    Color special_color_fg_hilite_color = Color16::Cyan;
    bool enable_special_colors = false;
    bool enable_4bit_colors = false;
    bool enable_rgb6_colors = false;
    bool enable_gray24_colors = false;
  };
  
  // /////////////////////////////////////////////////////////////
  //   _____      _            _____ _      _                   //
  //  / ____|    | |          |  __ (_)    | |                  //
  // | |     ___ | | ___  _ __| |__) |  ___| | _____ _ __       //
  // | |    / _ \| |/ _ \| '__|  ___/ |/ __| |/ / _ \ '__|      //
  // | |___| (_) | | (_) | |  | |   | | (__|   <  __/ |         //
  //  \_____\___/|_|\___/|_|  |_|   |_|\___|_|\_\___|_|         //
  // /////////////////////////////////////////////////////////////
  
  class ColorPicker : public Widget
  {
    Color fg_color_sel;
    Color fg_color_sel_hilite;
    ColorPickerParams params;
    char select_char;
    char unselect_char = ' ';
    RC caret = { 0, 0 };
    bool wrapping = false;
    static const int c_special_start = -3;
    static const int c_special_end = -1;
    static const int c_4bit_start = 0;
    static const int c_4bit_end = 15;
    static const int c_rgb6_start = 16;
    static const int c_rgb6_end = 231;
    static const int c_gray24_start = 232;
    static const int c_gray24_end = 255;
    int col_x_start = 0;
    int col_x_end = 0;
    int col_4b_start = 0;
    int col_4b_end = 0;
    int col_rgb6_start = 0;
    int col_rgb6_end = 0;
    int col_g24_start = 0;
    int col_g24_end = 0;
    int num_16color_colors = 0;
    int num_16color_rows = 0;
    int num_rgb6_rows = 0;
    int num_gray24_rows = 0;
    int num_rows = 0;
    int num_cols = 0;
    int r_max = 0;
    
    Color get_color(int idx) const
    {
      return Color(idx);
    }
    
  public:
    ColorPicker(Color fg_sel, Color fg_sel_hilite,
                const ColorPickerParams& cp_params,
                int tab = 0, bool cursor_wrapping = false,
                char sel_char = '*', char unsel_char = ' ',
                bool sel = false)
      : Widget(tab, sel)
      , fg_color_sel(fg_sel)
      , fg_color_sel_hilite(fg_sel_hilite)
      , params(cp_params)
      , select_char(sel_char)
      , unselect_char(unsel_char)
      , wrapping(cursor_wrapping)
      , col_x_start(params.enable_special_colors ? c_special_start : 0)
      , col_x_end(params.enable_special_colors ? c_special_end : -1)
      , col_4b_start(params.enable_4bit_colors ? c_4bit_start : 0)
      , col_4b_end(params.enable_4bit_colors ? c_4bit_end : -1)
      , col_rgb6_start(params.enable_rgb6_colors ? c_rgb6_start : 0)
      , col_rgb6_end(params.enable_rgb6_colors ? c_rgb6_end : -1)
      , col_g24_start(params.enable_gray24_colors ? c_gray24_start : 0)
      , col_g24_end(params.enable_gray24_colors ? c_gray24_end : -1)
    {
      if (params.enable_special_colors)
        num_16color_colors += 3;
      if (params.enable_4bit_colors)
        num_16color_colors += 16;
        
      if (params.enable_special_colors || params.enable_4bit_colors)
        num_16color_rows = 1;
      if (params.enable_rgb6_colors)
        num_rgb6_rows = 6;
      if (params.enable_gray24_colors)
        num_gray24_rows = 1;
        
      num_rows = std::max(1, num_16color_rows + num_rgb6_rows + num_gray24_rows);
      r_max = num_rows - 1;
      
      if (num_16color_colors > 0)
        num_cols = num_16color_colors;
      if (params.enable_rgb6_colors)
        num_cols = std::max(num_cols, 36);
      if (params.enable_gray24_colors)
        num_cols = std::max(num_cols, 24);
    }
    
    void update(SpecialKey curr_special_key)
    {
      if (!is_selected())
        return;
      
      bool moved = false;
      if (curr_special_key == SpecialKey::Up)
      {
        caret.r--;
        if (caret.r < 0)
          caret.r = wrapping ? r_max : 0;
        moved = true;
      }
      else if (curr_special_key == SpecialKey::Down)
      {
        caret.r++;
        if (caret.r > r_max)
          caret.r = wrapping ? 0 : r_max;
        moved = true;
      }
      else if (curr_special_key == SpecialKey::Left)
      {
        caret.c--;
        moved = true;
      }
      else if (curr_special_key == SpecialKey::Right)
      {
        caret.c++;
        moved = true;
      }
      
      if (moved)
      {
        if (num_16color_colors > 0 && caret.r == 0)
        {
          int c_max = num_16color_colors - 1;
          if (caret.c < 0)
            caret.c = wrapping ? c_max : 0;
          else if (caret.c > c_max)
            caret.c = wrapping ? 0 : c_max;
        }
        else if (params.enable_rgb6_colors && math::in_r_co(caret.r, num_16color_rows, num_16color_rows + num_rgb6_rows))
        {
          if (caret.c < 0)
            caret.c = wrapping ? 35 : 0; // 6*6-1
          else if (caret.c > 35)
            caret.c = wrapping ? 0 : 35;
        }
        else if (params.enable_gray24_colors && caret.r == num_16color_rows + num_rgb6_rows)
        {
          if (caret.c < 0)
            caret.c = wrapping ? 23 : 0;
          else if (caret.c > 23)
            caret.c = wrapping ? 0 : 23;
        }
      }
    }
    
    // TtD0123456789ABCDEF
    // ###### ###### ###### ###### ###### ######
    // ###### ###### ###### ###### ###### ######
    // ###### ###### ###### ###### ###### ######
    // ###### ###### ###### ###### ###### ######
    // ###### ###### ###### ###### ###### ######
    // ###### ###### ###### ###### ###### ######
    // GGGGGGGGGGGGGGGGGGGGGGGG
    
    template<int NR, int NC, typename CharT>
    void draw(ScreenHandler<NR, NC, CharT>& sh, const RC& pos, int anim_ctr) const
    {
      auto caret_bg_color = (params.enable_special_colors && caret.r == 0 && caret.c < -col_x_start) ? Color16::Black : get_color();
      const auto caret_bg_color_inv = t8::get_contrast_color(caret_bg_color);
      Color caret_fg_color_unsel = Color16::Default;
      switch (params.fg_cursor_coloring)
      {
        case ColorPickerCursorColoring::BlackWhite:
          caret_fg_color_unsel = t8::is_dark(caret_bg_color, true).value_or(false) ? Color16::White : Color16::Black;
          break;
        case ColorPickerCursorColoring::Contrast:
          caret_fg_color_unsel = caret_bg_color_inv;
          break;
      }
      if (params.enable_special_colors && caret.r == 0 && caret.c < -col_x_start)
        caret_fg_color_unsel = params.special_color_fg_hilite_color;
      auto caret_fg_color = anim_ctr == 0 ? (params.cursor_blinking == ColorPickerCursorBlinking::EveryOther ? caret_fg_color_unsel : fg_color_sel) : fg_color_sel_hilite;
      
      if (!is_selected())
        caret_fg_color = caret_fg_color_unsel;
      
      // Cursor
      if (!is_selected()
          || params.cursor_blinking == ColorPickerCursorBlinking::Persist
          || (anim_ctr == 0 && params.cursor_blinking == ColorPickerCursorBlinking::EveryOther))
      {
        char curr_char = select_char;
        if (!is_selected() && params.enable_special_colors && caret.r == 0 && 0 <= caret.c && caret.c <= -col_x_start)
        {
          if (caret.c == 0)
            curr_char = 'T';
          else if (caret.c == 1)
            curr_char = 't';
          else if (caret.c == 2)
            curr_char = 'D';
        }
        sh.write_buffer(curr_char, pos.r + caret.r, pos.c + caret.c, caret_fg_color, caret_bg_color);
      }
      
      // Colors
      for (int col = col_x_start; col <= col_x_end; ++col)
      {
        char curr_char = unselect_char;
        if (col == -3)
          curr_char = 'T';
        else if (col == -2)
          curr_char = 't';
        else if (col == -1)
          curr_char = 'D';
        sh.write_buffer(curr_char, pos.r, pos.c + col - col_x_start, Color16::White, Color16::Black);
      }
      for (int col = col_4b_start; col <= col_4b_end; ++col)
      {
        char curr_char = unselect_char;
        auto bg_color = get_color(col);
        sh.write_buffer(curr_char, pos.r, pos.c + col - col_x_start, Color16::White, bg_color);
      }
      for (int col = col_rgb6_start; col <= col_rgb6_end; ++col)
      {
        char curr_char = unselect_char;
        auto bg_color = get_color(col);
        auto rgb6 = bg_color.try_get_rgb6();
        if (!rgb6.has_value())
          continue;
        auto [r, g, b] = rgb6.value();
        sh.write_buffer(curr_char, pos.r + (num_16color_rows + g), pos.c + (6*r + b), Color16::White, bg_color);
      }
      for (int col = col_g24_start; col <= col_g24_end; ++col)
      {
        char curr_char = unselect_char;
        auto bg_color = get_color(col);
        auto gray24 = bg_color.try_get_gray24();
        if (!gray24.has_value())
          continue;
        auto [gray] = gray24.value();
        auto row_offs = num_16color_rows + num_rgb6_rows;
        sh.write_buffer(curr_char, pos.r + row_offs, pos.c + gray, Color16::White, bg_color);
      }
    }
    
    virtual int width() const override
    {
      return num_cols;
    }
    
    virtual int height() const override
    {
      return num_rows;
    }
    
    Color get_color() const
    {
      if ((params.enable_special_colors || params.enable_4bit_colors)
          && caret.r == 0)
        return Color(caret.c + col_x_start); // 0 <= x <= 15+3 -> -3 <= x <= 15
      if (params.enable_rgb6_colors && math::in_r_co(caret.r, num_16color_rows, num_16color_rows + num_rgb6_rows))
      {
        int r = caret.c / 6;
        int g = caret.r - num_16color_rows;
        int b = caret.c % 6;
        return Color(r, g, b);
      }
      if (params.enable_gray24_colors && caret.r == num_16color_rows + num_rgb6_rows)
        return Color(t8::Gray24(caret.c));
      return Color16::Default;
    }
    
    void set_color(Color color) // 254 (gray24:{22})
    {
      int idx = color.get_index();
      if (params.enable_special_colors && color.is_color16_special())
      {
        caret.c = idx - c_special_start;
        caret.r = 0;
      }
      else if (params.enable_4bit_colors && color.is_color16_regular())
      {
        caret.c = idx - (params.enable_special_colors ? c_special_start : 0);
        caret.r = 0;
      }
      else if (params.enable_rgb6_colors && color.is_rgb6())
      {
        auto [r, g, b] = color.try_get_rgb6().value();
        caret.c = r * 6 + b;
        caret.r = g + num_16color_rows;
      }
      else if (params.enable_gray24_colors && color.is_gray24())
      {
        auto [gray] = color.try_get_gray24().value();
        caret.c = gray;
        caret.r = num_16color_rows + num_rgb6_rows;
      }
      else
      {
        caret.c = 0;
        caret.r = 0;
      }
    }
    
    void clear()
    {
      caret = { 0, 0 };
    }
  };

}
