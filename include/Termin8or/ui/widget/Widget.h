//
//  Widget.h
//  Termin8or
//
//  Created by Rasmus Anthin on 2026-05-10.
//

#pragma once
#include <Core/MathUtils.h>

namespace t8x
{

  // /////////////////////////////////////////////////////////////
  // __          ___     _            _                         //
  // \ \        / (_)   | |          | |                        //
  //  \ \  /\  / / _  __| | __ _  ___| |_                       //
  //   \ \/  \/ / | |/ _` |/ _` |/ _ \ __|                      //
  //    \  /\  /  | | (_| | (_| |  __/ |_                       //
  //     \/  \/   |_|\__,_|\__, |\___|\__|                      //
  //                        __/ |                               //
  //                       |___/                                //
  // /////////////////////////////////////////////////////////////
  
  class Widget
  {
    int tab_order = 0;
    bool selected = false;
    
  public:
    virtual ~Widget() = default;
    Widget(int tab, bool sel = false)
      : tab_order(tab)
      , selected(sel)
    {}
    
    virtual int num_components() const { return 1; }
    virtual void set_component_focus(int /*sub_tab*/, bool /*selected*/) {}
  
    int get_tab_order() const { return tab_order; }
    bool try_tab_select(int tab, int sub_tab)
    {
      if (tab_order == tab)
      {
        selected = true;
        set_component_focus(sub_tab, true);
        return true;
      }
      return false;
    }
    
    virtual int width() const = 0;
    virtual int height() const = 0;
    
    virtual void set_selected(bool sel) { selected = sel; }
    void clear_focused_components()
    {
      for (int sc_idx = 0; sc_idx < num_components(); ++sc_idx)
        set_component_focus(sc_idx, false);
    }
    void toggle_selected() { math::toggle(selected); }
    bool is_selected() const { return selected; }
  };

}
