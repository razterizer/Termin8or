//
//  ButtonGroup.h
//  Termin8or
//
//  Created by Rasmus Anthin on 2026-05-10.
//

#pragma once
#include "Button.h"
#include <Core/StlUtils.h>
#include <Core/Utils.h>
#include <memory>

namespace t8x
{

  using RC = t8::RC;
  template<int NR, int NC, typename CharT>
  using ScreenHandler = t8::ScreenHandler<NR, NC, CharT>;

  // /////////////////////////////////////////////////////////////////
  //  ____        _   _               _____                         //
  // |  _ \      | | | |             / ____|                        //
  // | |_) |_   _| |_| |_ ___  _ __ | |  __ _ __ ___  _   _ _ __    //
  // |  _ <| | | | __| __/ _ \| '_ \| | |_ | '__/ _ \| | | | '_ \   //
  // | |_) | |_| | |_| || (_) | | | | |__| | | | (_) | |_| | |_) |  //
  // |____/ \__,_|\__|\__\___/|_| |_|\_____|_|  \___/ \__,_| .__/   //
  //                                                       | |      //
  //                                                       |_|      //
  // /////////////////////////////////////////////////////////////////
  
  class ButtonGroup
  {
    std::vector<std::unique_ptr<Button>> buttons;
    int selected = -1;
    
  public:
    bool empty() const { return buttons.empty(); }
    int size() const { return stlutils::sizeI(buttons); }
    
    void clear_selections()
    {
      for (auto& btn : buttons)
        btn->set_selected(false);
    }
    
    bool try_tab_select(int tab)
    {
      for (int b_idx = 0; b_idx < stlutils::sizeI(buttons); ++b_idx)
      {
        auto& btn = buttons[b_idx];
        if (btn->get_tab_order() == tab)
        {
          btn->set_selected(true);
          selected = b_idx;
          return true;
        }
      }
      return false;
    }
  
    Button* add(const Button& btn)
    {
      auto& uptr = buttons.emplace_back(std::make_unique<Button>(btn));
      return uptr.get();
    }
    
    template<typename... Args>
    Button* emplace(Args&&... args)
    {
      auto& uptr = buttons.emplace_back(std::make_unique<Button>(std::forward<Args>(args)...));
      return uptr.get();
    }
    
    void set_selection(int sel_idx, bool sel, int* sel_tab_idx)
    {
      selected = math::clamp(sel_idx, 0, static_cast<int>(buttons.size()));
      if (auto* button = get_selected_button(); button != nullptr)
      {
        button->set_selected(sel);
        if (sel)
          utils::try_set(sel_tab_idx, button->get_tab_order());
      }
    }
    Button* get_selected_button()
    {
      auto sz = stlutils::sizeI(buttons);
      if (0 <= selected && selected < sz)
        return buttons[selected].get();
      return nullptr;
    }
    void inc_selection(int* sel_tab_idx)
    {
      if (buttons.empty())
        return;
        
      auto* btn_pre = get_selected_button();
      if (btn_pre != nullptr)
        btn_pre->set_selected(false);
      
      selected++;
      if (selected >= static_cast<int>(buttons.size()))
        selected = 0;
        
      auto* btn_post = get_selected_button();
      if (btn_post != nullptr)
      {
        btn_post->set_selected(true);
        utils::try_set(sel_tab_idx, btn_post->get_tab_order());
      }
    }
    void dec_selection(int* sel_tab_idx)
    {
      if (buttons.empty())
        return;
        
      auto* btn_pre = get_selected_button();
      if (btn_pre != nullptr)
        btn_pre->set_selected(false);
      
      selected--;
      if (selected < 0)
        selected = static_cast<int>(buttons.size()) - 1;
        
      auto* btn_post = get_selected_button();
      if (btn_post != nullptr)
      {
        btn_post->set_selected(true);
        utils::try_set(sel_tab_idx, btn_post->get_tab_order());
      }
    }
    
    template<int NR, int NC, typename CharT>
    void draw(ScreenHandler<NR, NC, CharT>& sh, const RC& pos, int width) const
    {
      if (buttons.empty())
        return;
    
      int width_btns = stlutils::sum<int>(buttons, [](const auto& btn) { return btn->width(); });
      int spacing = (width - width_btns) / static_cast<int>(buttons.size() - (buttons.size() > 1));
      
      int c = pos.c;
      for (const auto& btn : buttons)
      {
        btn->draw(sh, RC { pos.r, c });
        c += btn->width() + spacing;
      }
    }
  };

}
