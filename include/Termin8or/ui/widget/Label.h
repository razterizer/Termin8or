//
//  Label.h
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
  using Style = t8::Style;
  template<int NR, int NC, typename CharT>
  using ScreenHandler = t8::ScreenHandler<NR, NC, CharT>;

  // /////////////////////////////////////////////////////////////
  //  _           _          _                                  //
  // | |         | |        | |                                 //
  // | |     __ _| |__   ___| |                                 //
  // | |    / _` | '_ \ / _ \ |                                 //
  // | |___| (_| | |_) |  __/ |                                 //
  // |______\__,_|_.__/ \___|_|                                 //
  // /////////////////////////////////////////////////////////////
  
  class Label : public Widget
  {
    std::string text;
    Style style;
    
  public:
    Label(const std::string& label_text, Style label_style,
          int tab = 0, bool sel = false)
      : Widget(tab, sel)
      , text(label_text)
      , style(label_style)
    {}
    
    template<int NR, int NC, typename CharT>
    void draw(ScreenHandler<NR, NC, CharT>& sh, const RC& pos) const
    {
      sh.write_buffer(text, pos, style);
    }
    
    virtual int width() const override
    {
      return static_cast<int>(text.length());
    }
    
    virtual int height() const override
    {
      return 1;
    }
    
    bool empty() const
    {
      return text.empty();
    }
  };

}
