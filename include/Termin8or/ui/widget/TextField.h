//
//  TextField.h
//  Termin8or
//
//  Created by Rasmus Anthin on 2026-05-10.
//

#pragma once
#include "Widget.h"
#include "../../geom/RC.h"
#include "../../input/KeyboardEnums.h"
#include "../../screen/Styles.h"
#include "../../screen/ScreenHandler.h"
#include <Core/StringHelper.h>
#include <string>


namespace t8x
{

  using RC = t8::RC;
  using SpecialKey = t8::SpecialKey;
  using PromptStyle = t8::PromptStyle;
  template<int NR, int NC, typename CharT>
  using ScreenHandler = t8::ScreenHandler<NR, NC, CharT>;

  enum class TextFieldMode { Numeric, AlphaNumeric, Alphabetic, Hex, PrintableAscii, All };

  // /////////////////////////////////////////////////////////////
  //  _______        _   ______ _      _     _                  //
  // |__   __|      | | |  ____(_)    | |   | |                 //
  //    | | _____  _| |_| |__   _  ___| | __| |                 //
  //    | |/ _ \ \/ / __|  __| | |/ _ \ |/ _` |                 //
  //    | |  __/>  <| |_| |    | |  __/ | (_| |                 //
  //    |_|\___/_/\_\\__|_|    |_|\___|_|\__,_|                 //
  // /////////////////////////////////////////////////////////////
  
  class TextField : public Widget
  {
    int field_width = 0;
    TextFieldMode mode = TextFieldMode::AlphaNumeric;
    PromptStyle style;
    char clear_char;
    std::string input;
    int caret = 0;
    
    int get_caret_pos() const { return std::min(field_width - 1, caret); }
    
    void add_char(char curr_key)
    {
      input[get_caret_pos()] = curr_key;
      caret++;
      if (caret > field_width)
        caret = field_width;
    }
    
    void backspace()
    {
      caret--;
      if (caret < 0)
        caret = 0;
      input[caret] = clear_char;
    }
    
  public:
    TextField(int width, TextFieldMode tf_mode, PromptStyle tf_style,
              int tab = 0, char clear_ch = '_', bool sel = false)
      : Widget(tab, sel)
      , field_width(width)
      , mode(tf_mode)
      , style(tf_style)
      , clear_char(clear_ch)
    {
      input = str::rep_char(clear_ch, width);
    }
  
    void update(char curr_key, SpecialKey curr_special_key)
    {
      if (!is_selected())
        return;
      if (mode == TextFieldMode::All && curr_key != 0)
        add_char(curr_key);
      else if (mode == TextFieldMode::PrintableAscii && 0x20 <= curr_key && curr_key <= 0x7E)
        add_char(curr_key);
      else if (str::is_digit(curr_key))
      {
        if (mode == TextFieldMode::Numeric || mode == TextFieldMode::AlphaNumeric || mode == TextFieldMode::Hex)
          add_char(curr_key);
      }
      else if (str::is_letter(curr_key))
      {
        if (mode == TextFieldMode::Alphabetic || mode == TextFieldMode::AlphaNumeric)
          add_char(curr_key);
        else if (mode == TextFieldMode::Hex)
        {
          auto ch_low = str::to_upper(curr_key);
          if ('A' <= ch_low && ch_low <= 'F')
            add_char(curr_key);
        }
      }
      else if (curr_special_key == SpecialKey::Backspace)
        backspace();
    }
    
    template<int NR, int NC, typename CharT>
    void draw(ScreenHandler<NR, NC, CharT>& sh, const RC& pos, int anim_ctr) const
    {
      if (is_selected())
      {
        auto bg_color_caret = (anim_ctr == 0) ? style.bg_color_cursor : style.bg_color;
        Color fg_color_caret = caret < field_width ? style.get_fg_color_clear() : style.fg_color;
        sh.write_buffer(input[get_caret_pos()], pos.r, pos.c + get_caret_pos(), fg_color_caret, bg_color_caret);
      }
    
      for (int i = 0; i < field_width; ++i)
      {
        char ch = input[i];
        Color fg_color = i >= caret && caret < field_width ? style.get_fg_color_clear() : style.fg_color;
        sh.write_buffer(ch, pos.r, pos.c + i, fg_color, style.bg_color);
      }
    }
    
    virtual int width() const override
    {
      return field_width;
    }
    
    virtual int height() const override
    {
      return 1;
    }
    
    const std::string get_input() const
    {
      return input.substr(0, caret);
    }
    
    void set_input(const std::string& str)
    {
      input = str::rep_char(clear_char, field_width);
      auto len = std::min(input.length(), str.length());
      input.replace(0, len, str.substr(0, len));
      caret = static_cast<int>(len);
    }
    
    void clear()
    {
      input = str::rep_char(clear_char, field_width);
      caret = 0;
    }
    
    bool empty() const
    {
      return caret == 0;
    }
  };

}
