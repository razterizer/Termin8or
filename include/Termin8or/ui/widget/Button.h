//
//  Button.h
//  Termin8or
//
//  Created by Rasmus Anthin on 2026-05-10.
//

#pragma once
#include "Widget.h"
#include "../../geom/RC.h"
#include "../../screen/Styles.h"
#include "../../screen/ScreenHandler.h"
#include <string>

namespace t8x
{
  using RC = t8::RC;
  using ButtonStyle = t8::ButtonStyle;
  template<int NR, int NC, typename CharT>
  using ScreenHandler = t8::ScreenHandler<NR, NC, CharT>;

  enum class ButtonFrame { None, Space, SquareBrackets, AngleBrackets, Braces, Parentheses, Pipes };

  // /////////////////////////////////////////////////////////////
  //  ____        _   _                                         //
  // |  _ \      | | | |                                        //
  // | |_) |_   _| |_| |_ ___  _ __                             //
  // |  _ <| | | | __| __/ _ \| '_ \                            //
  // | |_) | |_| | |_| || (_) | | | |                           //
  // |____/ \__,_|\__|\__\___/|_| |_|                           //
  // /////////////////////////////////////////////////////////////
  
  class Button : public Widget
  {
    std::string text;
    ButtonStyle style;
    ButtonFrame frame = ButtonFrame::SquareBrackets;
    
  public:
    Button(const std::string& txt, ButtonStyle btn_style, ButtonFrame btn_frame, int tab = 0, bool sel = false)
      : Widget(tab, sel)
      , text(txt)
      , style(btn_style)
      , frame(btn_frame)
    {}
    
    virtual int width() const override
    {
      int left_right = 2;
      if (frame == ButtonFrame::None)
        left_right = 0;
      return left_right + static_cast<int>(text.size());
    }
    
    virtual int height() const override
    {
      return 1;
    }
    
    template<int NR, int NC, typename CharT>
    void draw(ScreenHandler<NR, NC, CharT>& sh, const RC& pos) const
    {
      std::string left = "";
      std::string right = "";
      switch (frame)
      {
        case ButtonFrame::None:
          break;
        case ButtonFrame::Space:
          left = " ";
          right = " ";
          break;
        case ButtonFrame::SquareBrackets:
          left = "[";
          right = "]";
          break;
        case ButtonFrame::AngleBrackets:
          left = "<";
          right = ">";
          break;
        case ButtonFrame::Braces:
          left = "{";
          right = "}";
          break;
        case ButtonFrame::Parentheses:
          left = "(";
          right = ")";
          break;
        case ButtonFrame::Pipes:
          left = "|";
          right = "|";
          break;
      }
      sh.write_buffer(left + text + right, pos, style.fg_color, is_selected() ? style.bg_color_selected : style.bg_color);
    }
    
    const std::string get_text() const { return text; }
  };

}
