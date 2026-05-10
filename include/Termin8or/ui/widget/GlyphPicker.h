//
//  GlyphPicker.h
//  Termin8or
//
//  Created by Rasmus Anthin on 2026-05-10.
//

#pragma once
#include "Widget.h"
#include "Label.h"
#include "TextField.h"
#include "../../geom/RC.h"
#include "../../input/KeyboardEnums.h"
#include "../../screen/Styles.h"
#include "../../screen/ScreenHandler.h"
#include "../../screen/Glyph.h"
#include "../../screen/StyledString.h"
#include "../../screen/TermHelper.h"
#include <string>
#include <array>

namespace t8x
{

  using RC = t8::RC;
  using SpecialKey = t8::SpecialKey;
  using Color = t8::Color;
  using Style = t8::Style;
  using PromptStyle = t8::PromptStyle;
  template<int NR, int NC, typename CharT>
  using ScreenHandler = t8::ScreenHandler<NR, NC, CharT>;

  // /////////////////////////////////////////////////////////////
  //   _____ _             _     _____ _      _                 //
  //  / ____| |           | |   |  __ (_)    | |                //
  // | |  __| |_   _ _ __ | |__ | |__) |  ___| | _____ _ __     //
  // | | |_ | | | | | '_ \| '_ \|  ___/ |/ __| |/ / _ \ '__|    //
  // | |__| | | |_| | |_) | | | | |   | | (__|   <  __/ |       //
  //  \_____|_|\__, | .__/|_| |_|_|   |_|\___|_|\_\___|_|       //
  //            __/ | |                                         //
  //           |___/|_|                                         //
  // /////////////////////////////////////////////////////////////
  
  class GlyphPicker : public Widget
  {
    // cp_ : codepoint
    // fb_ : fallback
    // cg_ : current glyph
    // rg_ : recent glyphs
    Color rg_sel_bg_color;
    Style lbl_style;
    Style brck_style;
    Label rg_lbl;
    Label cg_lbl;
    Label cp_lbl;
    Label fb_lbl;
    Label hex_prefix_lbl;
    TextField cp_hex_field; // preferred in hex (UTF-8)
    TextField fb_hex_field; // fallback in hex (ASCII)
    TextField fb_field; // fallback (ASCII)
    std::string cp_str_prev;
    std::string fb_str_prev;
    
    std::array<t8::Glyph, 4> recent_glyphs;
    int sel_recent_idx = -1; // Current selection in newest-first display order.
    int recent_count = 0; // Number of valid recent entries currently stored.
    
    t8::Glyph current_glyph;
    mutable std::vector<t8::StyledString> current_glyph_disp_sstr_long; // Cached representation of current_glyph for display.
    mutable std::vector<t8::StyledString> current_glyph_disp_sstr_short; // Cached representation of current_glyph for display.
    
    t8::Glyph get_canonicalized_glyph() const
    {
      auto glyph = current_glyph;
      glyph.try_canonicalize_from_fallback();
      return glyph;
    }
    
    void move_recent_selection(int offs_idx)
    {
      if (recent_count == 0)
      {
        sel_recent_idx = -1;
        return;
      }
      sel_recent_idx = math::clamp(sel_recent_idx + offs_idx, 0, recent_count - 1);
    }
    
    void set_glyph_inputs(const t8::Glyph& g)
    {
      if (g.preferred == t8::Glyph::none32)
        cp_hex_field.clear();
      else
        cp_hex_field.set_input(str::int2hex(g.preferred));
      
      if (g.fallback == t8::Glyph::none)
      {
        fb_hex_field.clear();
        fb_field.clear();
      }
      else
      {
        fb_hex_field.set_input(str::int2hex(g.fallback));
        fb_field.set_input(std::string(1, g.fallback));
      }
      
      cp_str_prev = cp_hex_field.get_input();
      fb_str_prev = fb_field.get_input();
    }
    
  public:
    GlyphPicker(PromptStyle tf_style, Style label_style, Style hex_prefix_style, Style bracket_style,
                Color recent_glyph_sel_bg_color,
                int tab, char clear_ch = '_', bool sel = false)
      : Widget(tab, sel)
      , rg_sel_bg_color(recent_glyph_sel_bg_color)
      , lbl_style(label_style)
      , brck_style(bracket_style)
      , rg_lbl("Recent Glyphs:", label_style)
      , cg_lbl("Current Glyph:", label_style)
      , cp_lbl("Preferred (Unicode):", label_style)
      , fb_lbl("Fallback (ASCII):", label_style)
      , hex_prefix_lbl("0x", hex_prefix_style)
      , cp_hex_field(6, TextFieldMode::Hex, tf_style,
                 tab, clear_ch, sel)
      , fb_hex_field(2, TextFieldMode::Hex, tf_style,
                 tab, clear_ch, sel)
      , fb_field(1, TextFieldMode::PrintableAscii, tf_style,
                 tab, clear_ch, sel)
    {}
  
    void set_glyph(const t8::Glyph& g)
    {
      current_glyph = g;
      set_glyph_inputs(g);
    }
    
    t8::Glyph get_glyph() const
    {
      return get_canonicalized_glyph();
    }
    
    const std::vector<t8::StyledString>& get_glyph_sstr_long() const
    {
      return current_glyph_disp_sstr_long;
    }
    
    const std::vector<t8::StyledString>& get_glyph_sstr_short() const
    {
      return current_glyph_disp_sstr_short;
    }
    
    void update(char curr_key, SpecialKey curr_special_key)
    {
      if (!is_selected())
        return;
      
      if (sel_recent_idx != -1)
      {
        if (curr_special_key == SpecialKey::Left)
          move_recent_selection(-1);
        else if (curr_special_key == SpecialKey::Right)
          move_recent_selection(+1);
        else if (curr_key == ' ')
          set_glyph(recent_glyphs[sel_recent_idx]);
      }
        
      if (cp_hex_field.is_selected())
      {
        cp_hex_field.update(curr_key, curr_special_key);
        if (cp_hex_field.empty())
          current_glyph.preferred = t8::Glyph::none32;
        else if (cp_str_prev != cp_hex_field.get_input())
          current_glyph.preferred = str::hex2int(cp_hex_field.get_input());
        cp_str_prev = cp_hex_field.get_input();
      }
      
      auto f_handle_fb_field = [&]()
      {
        if (fb_field.empty())
          current_glyph.fallback = t8::Glyph::none;
        else if (fb_str_prev != fb_field.get_input())
          current_glyph.fallback = fb_field.get_input()[0];
        fb_str_prev = fb_field.get_input();
      };
      
      if (fb_hex_field.is_selected())
      {
        fb_hex_field.update(curr_key, curr_special_key);
        auto hex_ch = static_cast<char>(str::hex2int(fb_hex_field.get_input()));
        if (t8::term::is_printable_ascii(hex_ch))
          fb_field.set_input(std::string(1, hex_ch));
        else
          fb_field.clear();
        f_handle_fb_field();
      }
      if (fb_field.is_selected())
      {
        fb_field.update(curr_key, curr_special_key);
        auto str = fb_field.get_input();
        if (!str.empty())
          fb_hex_field.set_input(str::int2hex(str[0]));
        f_handle_fb_field();
      }
    }
    
    template<int NR, int NC, typename CharT>
    void draw(ScreenHandler<NR, NC, CharT>& sh, const RC& pos, int anim_ctr) const
    {
      // Current Glyph: [|]
      // Recent Glyphs: [#8][pf][pf][pf]
      // Preferred (Unicode): 0x______
      // Fallback (ASCII):      _
      
      //sh.write_buffer("Recent Glyphs:", pos, Color)
      
      const auto disp_glyph = get_canonicalized_glyph();
      current_glyph_disp_sstr_long = disp_glyph.format_long<CharT>(!disp_glyph.empty(), true, lbl_style, lbl_style, brck_style);
      current_glyph_disp_sstr_short = disp_glyph.format_short<CharT>(true, lbl_style, lbl_style, brck_style);
      
      // /////
      
      cg_lbl.draw(sh, pos + RC { 0, 0 });
      sh.write_buffer(current_glyph_disp_sstr_long, pos + RC { 0, cg_lbl.width() + 1 });
      
      rg_lbl.draw(sh, pos + RC { 1, 0 });
      {
        int rg_col_start = rg_lbl.width() + 1;
        int rg_col_offs = 0;
        for (int g_idx = 0; g_idx < recent_count; ++g_idx)
        {
          const auto& rg = recent_glyphs[g_idx];
          auto glyph_style = lbl_style;
          auto bracket_style = brck_style;
          if (g_idx == sel_recent_idx)
          {
            glyph_style.bg_color = rg_sel_bg_color;
            bracket_style.bg_color = rg_sel_bg_color;
          }
          auto ss_vec = rg.format_short<CharT>(true, glyph_style, glyph_style, bracket_style);
          rg_col_offs = sh.write_buffer(ss_vec, pos + RC { 1, rg_col_start + rg_col_offs });
        }
      }
      
      cp_lbl.draw(sh, pos + RC { 2, 0 });
      hex_prefix_lbl.draw(sh, pos + RC { 2, cp_lbl.width() + 1 });
      cp_hex_field.draw(sh, pos + RC { 2, cp_lbl.width() + 1 + hex_prefix_lbl.width() }, anim_ctr);
      
      fb_lbl.draw(sh, pos + RC { 3, 0 });
      hex_prefix_lbl.draw(sh, pos + RC { 3, cp_lbl.width() + 1 });
      fb_hex_field.draw(sh, pos + RC { 3, cp_lbl.width() + 1 + hex_prefix_lbl.width() }, anim_ctr);
      fb_field.draw(sh, pos + RC { 3, cp_lbl.width() + 1 + hex_prefix_lbl.width() + 3 + fb_hex_field.width() }, anim_ctr);
    }
    
    virtual int width() const override
    {
      return cp_lbl.width() + 1 + hex_prefix_lbl.width() + cp_hex_field.width();
    }
    
    virtual int height() const override
    {
      return 4;
    }
    
    void clear()
    {
      current_glyph.clear();
      current_glyph_disp_sstr_long.clear();
      current_glyph_disp_sstr_short.clear();
      cp_hex_field.clear();
      fb_hex_field.clear();
      fb_field.clear();
      cp_str_prev.clear();
      fb_str_prev.clear();
    }
    
    bool empty() const
    {
      return get_canonicalized_glyph().fully_empty();
    }
    
    // Manual validation matrix:
    // []         => reject as empty.
    // [|a]       => accept, canonicalize to [a|].
    // [a|a]/[a|] => accept.
    // [2603|]    => reject; Unicode glyphs need ASCII fallback.
    // [2603|*]   => accept.
    // Recents    => no empty, no duplicates, canonicalized entries only.
    bool valid() const
    {
      return current_glyph.valid_after_canonicalization();
    }
    
    virtual int num_components() const override { return 4; }
    
    virtual void set_component_focus(int sub_tab, bool selected) override
    {
      if (sub_tab == 0)
        sel_recent_idx = selected && recent_count > 0 ? std::max(0, sel_recent_idx) : -1;
      else if (sub_tab == 1)
        cp_hex_field.set_selected(selected);
      else if (sub_tab == 2)
        fb_hex_field.set_selected(selected);
      else if (sub_tab == 3)
        fb_field.set_selected(selected);
    }
    
    // Recent glyphs newest-first MRU mechanics.
    // push A -> [A]
    // push B -> [B, A]
    // push C -> [C, B, A]
    // push D -> [D, C, B, A]
    // push E -> [E, D, C, B]   // A falls off.
    // push C -> [C, E, D, B]   // C promoted, no duplicate.
    void push_recent()
    {
      const auto glyph = get_canonicalized_glyph();
      if (glyph.fully_empty())
        return;
      if (!glyph.valid())
        return;
      
      // Attempting to find the index to an already existing item matching curr glyph.
      // [D, C, B, A] | push E -> [E, D, C, B] : existing_idx = -1
      // [E, D, C, B] | push C -> [C, E, D, B] : existing_idx = 2
      int existing_idx = -1;
      for (int recent_idx = 0; recent_idx < recent_count; ++recent_idx)
      {
        if (recent_glyphs[recent_idx] == glyph)
        {
          existing_idx = recent_idx;
          break;
        }
      }
      
      // Calculating the number of glyphs in the list to move, clamped to the length of the list.
      // [D, C, B, A] | push E -> [E, D, C, B] : move_count = 3
      // [E, D, C, B] | push C -> [C, E, D, B] : move_count = 2
      const int move_count = existing_idx == -1
        ? std::min(recent_count, stlutils::sizeI(recent_glyphs) - 1)
        : existing_idx;
      
      // Moving older glyphs towards the back.
      // [D, C, B, A] -> [D, C, B, B] -> [D, C, C, B] -> [D, D, C, B]
      // [E, D, C, B] -> [E, D, D, B] -> [E, E, D, B]
      for (int recent_idx = move_count; recent_idx > 0; --recent_idx)
        recent_glyphs[recent_idx] = recent_glyphs[recent_idx - 1];
      
      // Inserting the new glyph.
      // [E*, D, C, B]
      // [C*, E, D, B]
      recent_glyphs[0] = glyph;
      
      // Bumping recent count clamped to the length of the list.
      if (existing_idx == -1)
        recent_count = std::min(recent_count + 1, stlutils::sizeI(recent_glyphs));
      
      // Reset selection index to 0 if we already have a selection.
      if (sel_recent_idx != -1)
        sel_recent_idx = 0;
    }
  };

}
