//
//  Dialog.h
//  Termin8or
//
//  Created by Rasmus Anthin on 2026-05-10.
//

#pragma once
#include "ButtonGroup.h"
#include "Label.h"
#include "TextField.h"
#include "GlyphPicker.h"
#include "ColorPicker.h"
#include "TextBox.h"
#include <memory>
#include <vector>
#include <tuple>

namespace t8x
{

  using RC = t8::RC;
  using SpecialKey = t8::SpecialKey;
  using Style = t8::Style;
  using PromptStyle = t8::PromptStyle;
  template<int NR, int NC, typename CharT>
  using ScreenHandler = t8::ScreenHandler<NR, NC, CharT>;

  // /////////////////////////////////////////////////////////////
  //  _____  _       _                                          //
  // |  __ \(_)     | |                                         //
  // | |  | |_  __ _| | ___   __ _                              //
  // | |  | | |/ _` | |/ _ \ / _` |                             //
  // | |__| | | (_| | | (_) | (_| |                             //
  // |_____/|_|\__,_|_|\___/ \__, |                             //
  //                          __/ |                             //
  //                         |___/                              //
  // /////////////////////////////////////////////////////////////
  
  // #NOTE: Supported template argument types: std::string and t8::GlyphString.
  template<typename StrT = std::string>
  class Dialog : public TextBox<StrT>
  {
    std::vector<std::tuple<RC, Style, t8::Glyph>> override_textels_pre;
    std::vector<std::pair<RC, std::vector<t8::StyledString>>> override_sstr_vecs_pre;
    ButtonGroup button_group; // Buttons have their own reserved row two rows down.
    std::vector<std::pair<RC, std::unique_ptr<Label>>> labels;
    std::vector<std::pair<RC, std::unique_ptr<TextField>>> text_fields;
    std::vector<std::pair<RC, std::unique_ptr<GlyphPicker>>> glyph_pickers;
    std::vector<std::pair<RC, std::unique_ptr<ColorPicker>>> color_pickers;
    std::vector<Widget*> all_widgets;
    int tab_idx = 0;
    int sub_tab_idx = 0;
    int max_tab_idx = 0;
    
    virtual bool has_buttons() const override { return !button_group.empty(); }
    
    void init()
    {
      TextBox<StrT>::init();
    }
    
    // Resizes the dialog depending on its contents.
    virtual RC measure_panel_size() const override
    {
      auto panel_size = TextBox<StrT>::measure_panel_size();
      
      for (const auto& lp : labels)
      {
        math::maximize(panel_size.r, lp.first.r + lp.second->height());
        math::maximize(panel_size.c, lp.first.c + lp.second->width());
      }
        
      for (const auto& tfp : text_fields)
      {
        math::maximize(panel_size.r, tfp.first.r + tfp.second->height());
        math::maximize(panel_size.c, tfp.first.c + tfp.second->width());
      }
        
      for (const auto& gpp : glyph_pickers)
      {
        math::maximize(panel_size.r, gpp.first.r + gpp.second->height());
        math::maximize(panel_size.c, gpp.first.c + gpp.second->width());
      }
        
      for (const auto& cpp : color_pickers)
      {
        math::maximize(panel_size.r, cpp.first.r + cpp.second->height());
        math::maximize(panel_size.c, cpp.first.c + cpp.second->width());
      }
      
      return panel_size;
    }
    
  public:
    Dialog() = default;
    Dialog(str::Adjustment master_adj)
      : TextBox<StrT>(master_adj)
    {}
    Dialog(size_t num_lines, str::Adjustment master_adj = str::Adjustment::Left)
      : TextBox<StrT>(num_lines, master_adj)
    {}
    Dialog(const std::vector<StrT>& text_lines, const std::vector<Style>& styles = {},
           str::Adjustment master_adj = str::Adjustment::Left)
      : TextBox<StrT>(text_lines, styles, master_adj)
    {}
    Dialog(const StrT& text, str::Adjustment master_adj = str::Adjustment::Left)
      : TextBox<StrT>(text, master_adj)
    {}
    
    void clear_selections()
    {
      button_group.clear_selections();
      for (auto& tfp : text_fields)
        tfp.second->set_selected(false);
      for (auto& gpp : glyph_pickers)
      {
        gpp.second->set_selected(false);
        gpp.second->clear_focused_components();
      }
      for (auto& cpp : color_pickers)
        cpp.second->set_selected(false);
    }
    
    void set_tab_selection(int tab, int sub_tab = 0)
    {
      // Clear widgets
      clear_selections();
      
      // Set current indices
      tab_idx = tab;
      sub_tab_idx = sub_tab;
      
      // Select a widget
      if (button_group.try_tab_select(tab))
        return;
      for (auto& tfp : text_fields)
        if (tfp.second->try_tab_select(tab, sub_tab))
          return;
      for (auto& gpp : glyph_pickers)
        if (gpp.second->try_tab_select(tab, sub_tab))
          return;
      for (auto& cpp : color_pickers)
        if (cpp.second->try_tab_select(tab, sub_tab))
          return;
    }
    
    void get_tab_selection(int& tab, int& sub_tab)
    {
      tab = tab_idx;
      sub_tab = sub_tab_idx;
    }
    
    void set_textel_pre(const RC& local_pos, const t8::Glyph& g, Color fg_color, Color bg_color)
    {
      //stlutils::emplace_back_if_not(override_textels_pre,
      //  std::tuple<RC, Style, char> { local_pos, { fg_color, bg_color }, ch},
      //  [&local_pos](const auto& otp) { return std::get<0>(otp) == local_pos; });
      auto it = stlutils::find_if(override_textels_pre,
                  [&local_pos](const auto& otp) { return std::get<0>(otp) == local_pos; });
      if (it != override_textels_pre.end())
      {
        std::get<1>(*it) = { fg_color, bg_color };
        std::get<2>(*it) = g;
      }
      else
        override_textels_pre.emplace_back(local_pos, Style { fg_color, bg_color }, g);
    }
    
    void set_textel_str_pre(const RC& local_pos, std::string_view str, Color fg_color, Color bg_color)
    {
      auto len = str::lenI(str);
      for (int i = 0; i < len; ++i)
        set_textel_pre(local_pos + RC { 0, i }, str[i], fg_color, bg_color);
    }
    
    void clear_textel_pre(const RC& local_pos)
    {
      stlutils::erase_if(override_textels_pre, [&local_pos](const auto& otp) { return std::get<0>(otp) == local_pos; });
    }
    
    void clear_all_textel_pre()
    {
      override_textels_pre.clear();
    }
    
    void set_sstr_vec_pre(const RC& local_pos, const std::vector<t8::StyledString>& sstr_vec)
    {
      auto it = stlutils::find_if(override_sstr_vecs_pre,
                                  [&local_pos](const auto& otp) { return otp.first == local_pos; });
      if (it != override_sstr_vecs_pre.end())
        it->second = sstr_vec;
      else
        override_sstr_vecs_pre.emplace_back(local_pos, sstr_vec);
    }
    
    void clear_sstr_vec_pre(const RC& local_pos)
    {
      stlutils::erase_if(override_sstr_vecs_pre, [&local_pos](const auto& otp) { return std::get<0>(otp) == local_pos; });
    }
    
    void clear_all_sstr_vec_pre()
    {
      override_sstr_vecs_pre.clear();
    }
    
    Button& emplace_button(const std::string& txt, ButtonStyle btn_style, ButtonFrame btn_frame, int tab = 0, bool sel = false)
    {
      auto* rptr = button_group.emplace(txt, btn_style, btn_frame, tab, sel);
      all_widgets.emplace_back(rptr);
      math::maximize(max_tab_idx, rptr->get_tab_order());
      return *rptr;
    }
    
    void set_button_selection(int sel_idx, bool sel)
    {
      button_group.clear_selections();
      button_group.set_selection(sel_idx, sel, &tab_idx);
    }
    
    void inc_button_selection()
    {
      button_group.inc_selection(&tab_idx);
    }
    
    void dec_button_selection()
    {
      button_group.dec_selection(&tab_idx);
    }
    
    std::string get_selected_button_text()
    {
      if (auto* button = button_group.get_selected_button(); button != nullptr)
        return button->get_text();
      return "";
    }
    
    Label& emplace_label(const RC& pos,
                         const std::string& label_text, Style label_style)
    {
      int tab = -1;
      bool sel = false;
      auto uptr = std::make_unique<Label>(label_text, label_style, tab, sel);
      auto* rptr = uptr.get();
      labels.emplace_back(pos, std::move(uptr));
      all_widgets.emplace_back(rptr);
      return *rptr;
    }
    
    TextField& emplace_text_field(const RC& pos,
                                  int width, TextFieldMode tf_mode, PromptStyle tf_style,
                                  int tab = 0, char clear_ch = '_', bool sel = false)
    {
      auto uptr = std::make_unique<TextField>(width, tf_mode, tf_style, tab, clear_ch, sel);
      auto* rptr = uptr.get();
      text_fields.emplace_back(pos, std::move(uptr));
      all_widgets.emplace_back(rptr);
      math::maximize(max_tab_idx, rptr->get_tab_order());
      return *rptr;
    }
    
    const std::string get_text_field_input(int tab) const
    {
      auto it = stlutils::find_if(text_fields, [tab](const auto& tfp) { return tfp.second->get_tab_order() == tab; });
      if (it != text_fields.end())
        return it->second->get_input();
      return "";
    }
    
    void set_text_field_input(int tab, const std::string& str)
    {
      auto it = stlutils::find_if(text_fields, [tab](const auto& tfp) { return tfp.second->get_tab_order() == tab; });
      if (it != text_fields.end())
        return it->second->set_input(str);
    }
    
    void clear_text_field(int tab)
    {
      auto it = stlutils::find_if(text_fields, [tab](const auto& tfp) { return tfp.second->get_tab_order() == tab; });
      if (it != text_fields.end())
        return it->second->clear();
    }
    
    void clear_all_text_fields()
    {
      for (auto& tf : text_fields)
        tf.second->clear();
    }
    
    bool text_field_empty(int tab)
    {
      auto it = stlutils::find_if(text_fields, [tab](const auto& tfp) { return tfp.second->get_tab_order() == tab; });
      if (it != text_fields.end())
        return it->second->empty();
      return true; // Treat as empty if unable to find matching text field.
    }
    
    GlyphPicker& emplace_glyph_picker(const RC& pos,
                                      PromptStyle tf_style, Style label_style, Style hex_prefix_style, Style bracket_style,
                                      Color recent_glyph_sel_bg_color,
                                      int tab, char clear_ch = '_', bool sel = false)
    {
      auto uptr = std::make_unique<GlyphPicker>(tf_style, label_style, hex_prefix_style, bracket_style,
                                                recent_glyph_sel_bg_color, tab, clear_ch, sel);
      auto* rptr = uptr.get();
      glyph_pickers.emplace_back(pos, std::move(uptr));
      all_widgets.emplace_back(rptr);
      math::maximize(max_tab_idx, rptr->get_tab_order());
      return *rptr;
    }
    
    t8::Glyph get_glyph_picker_glyph(int tab) const
    {
      auto it = stlutils::find_if(glyph_pickers, [tab](const auto& gpp) { return gpp.second->get_tab_order() == tab; });
      if (it != glyph_pickers.end())
        return it->second->get_glyph();
      return {};
    }
    
    void set_glyph_picker_glyph(int tab, const t8::Glyph& g)
    {
      auto it = stlutils::find_if(glyph_pickers, [tab](const auto& gpp) { return gpp.second->get_tab_order() == tab; });
      if (it != glyph_pickers.end())
        return it->second->set_glyph(g);
    }
    
    std::vector<t8::StyledString> get_glyph_picker_disp_sstr(int tab, bool short_format) const
    {
      auto it = stlutils::find_if(glyph_pickers, [tab](const auto& gpp) { return gpp.second->get_tab_order() == tab; });
      if (it != glyph_pickers.end())
        return short_format ? it->second->get_glyph_sstr_short() : it->second->get_glyph_sstr_long();
      return {};
    }
    
    void clear_glyph_picker(int tab)
    {
      auto it = stlutils::find_if(glyph_pickers, [tab](const auto& gpp) { return gpp.second->get_tab_order() == tab; });
      if (it != glyph_pickers.end())
        return it->second->clear();
    }
    
    bool glyph_picker_empty(int tab)
    {
      auto it = stlutils::find_if(glyph_pickers, [tab](const auto& gpp) { return gpp.second->get_tab_order() == tab; });
      if (it != glyph_pickers.end())
        return it->second->empty();
      return true; // Treat as empty if unable to find matching glyph picker.
    }
    
    bool glyph_picker_valid(int tab)
    {
      auto it = stlutils::find_if(glyph_pickers, [tab](const auto& gpp) { return gpp.second->get_tab_order() == tab; });
      if (it != glyph_pickers.end())
        return it->second->valid();
      return false;
    }
    
    ColorPicker& emplace_color_picker(const RC& pos,
                                      Color fg_sel, Color fg_sel_hilite,
                                      const ColorPickerParams& cp_params,
                                      int tab = 0, bool cursor_wrapping = false,
                                      char sel_char = '*', char unsel_char = ' ',
                                      bool sel = false)
    {
      auto uptr = std::make_unique<ColorPicker>(fg_sel, fg_sel_hilite, cp_params, tab, cursor_wrapping, sel_char, unsel_char, sel);
      auto* rptr = uptr.get();
      color_pickers.emplace_back(pos, std::move(uptr));
      all_widgets.emplace_back(rptr);
      math::maximize(max_tab_idx, rptr->get_tab_order());
      return *rptr;
    }
    
    const Color get_color_picker_color(int tab) const
    {
      auto it = stlutils::find_if(color_pickers, [tab](const auto& cpp) { return cpp.second->get_tab_order() == tab; });
      if (it != color_pickers.end())
        return it->second->get_color();
      return Color16::Default;
    }
    
    void set_color_picker_color(int tab, Color color)
    {
      auto it = stlutils::find_if(color_pickers, [tab](const auto& cpp) { return cpp.second->get_tab_order() == tab; });
      if (it != color_pickers.end())
        return it->second->set_color(color);
    }
    
    void clear_color_picker(int tab)
    {
      auto it = stlutils::find_if(color_pickers, [tab](const auto& cpp) { return cpp.second->get_tab_order() == tab; });
      if (it != color_pickers.end())
        return it->second->clear();
    }
    
    void update(char curr_key, SpecialKey curr_special_key)
    {
      if (curr_special_key == SpecialKey::Tab)
      {
        // Label(-1), TextField(0), Label(-1), GlyphPicker(1), TextField(2) :
        // 0.0 -> 1.0
        // 1.0 -> 1.1
        // 1.1 -> 2.0
        // GlyphPicker(0), Label(-1), ColorPicker(1)
        // 0.0 -> 0.1
        // 0.1 -> 1.0
        auto it = stlutils::find_if(all_widgets, [&](const auto* widget) { return widget->get_tab_order() == tab_idx; });
        if (it != all_widgets.end() && (*it)->get_tab_order() != -1)
        {
          sub_tab_idx = (sub_tab_idx + 1) % (*it)->num_components();
          if (sub_tab_idx == 0)
            tab_idx = (tab_idx + 1) % (max_tab_idx + 1);
        }
        else
          tab_idx = (tab_idx + 1) % (max_tab_idx + 1);
        
        set_tab_selection(tab_idx, sub_tab_idx);
      }
      else if (curr_special_key == SpecialKey::Left)
        dec_button_selection();
      else if (curr_special_key == SpecialKey::Right)
        inc_button_selection();

      for (auto& tfp : text_fields)
        tfp.second->update(curr_key, curr_special_key);
      for (auto& gpp : glyph_pickers)
        gpp.second->update(curr_key, curr_special_key);
      for (auto& cpp : color_pickers)
        cpp.second->update(curr_special_key);
    }
    
    template<int NR, int NC, typename CharT>
    void draw(ScreenHandler<NR, NC, CharT>& sh, const TextBoxDrawingArgsPos& args, int anim_ctr)
    {
      const auto& pos = args.pos;
      
      auto panel_size = TextBox<StrT>::fetch_panel_size();
      int r_len = panel_size.r;
      int c_len = panel_size.c;
      
      for (const auto& [tp, style, glyph] : override_textels_pre)
      {
        if (math::in_range(tp.r, 0, r_len, Range::ClosedOpen) &&
            math::in_range(tp.c, 0, c_len, Range::ClosedOpen))
        {
          sh.write_buffer(glyph,
                          pos.r + tp.r, pos.c + tp.c,
                          style);
        }
      }
      
      for (const auto& [tp, ss_vec] : override_sstr_vecs_pre)
      {
        auto width = t8::get_sstr_vec_width(ss_vec);
        if (math::in_range(tp.r, 0, r_len, Range::ClosedOpen) &&
            math::in_range(tp.c, 0, c_len - width, Range::Closed))
        {
          sh.write_buffer(ss_vec,
                          pos.r + tp.r, pos.c + tp.c);
        }
      }
                  
      button_group.draw(sh,
                        { pos.r + r_len + 1, pos.c },
                        c_len);
      
      for (const auto& lp : labels)
        lp.second->draw(sh, pos + lp.first);
        
      for (const auto& tfp : text_fields)
        tfp.second->draw(sh, pos + tfp.first, anim_ctr);
        
      for (const auto& gpp : glyph_pickers)
        gpp.second->draw(sh, pos + gpp.first, anim_ctr);
        
      for (const auto& cpp : color_pickers)
        cpp.second->draw(sh, pos + cpp.first, anim_ctr);
        
      TextBox<StrT>::draw(sh, args);
    }
    
    template<int NR, int NC, typename CharT>
    void draw(ScreenHandler<NR, NC, CharT>& sh, const TextBoxDrawingArgsAlign& args, int anim_ctr)
    {
      auto pargs = TextBox<StrT>::template get_drawing_args_pos<NR, NC>(args);
      
      draw(sh, pargs, anim_ctr);
    }
  };

}
