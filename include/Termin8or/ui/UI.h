//
//  UI.h
//  Termin8or
//
//  Created by Rasmus Anthin on 2024-07-08.
//

#pragma once
#include "../geom/RC.h"
#include "../input/KeyboardEnums.h"
#include "../drawing/Drawing.h"
#include "../screen/StyledString.h"
#include <Core/StringBox.h>
#include <Core/Utils.h>

#define PARAM(var) #var, &var

namespace t8x
{
  using Style = t8::Style;
  using ButtonStyle = t8::ButtonStyle;
  using PromptStyle = t8::PromptStyle;
  using RC = t8::RC;
  template<int NR, int NC, typename CharT>
  using ScreenHandler = t8::ScreenHandler<NR, NC, CharT>;
  using SpecialKey = t8::SpecialKey;
  

  enum class VerticalAlignment { TOP, CENTER, BOTTOM };
  enum class HorizontalAlignment { LEFT, CENTER, RIGHT };
  
  struct TextBoxDrawingArgs
  {
    Style box_style;
    bool draw_box_outline = true;
    bool draw_box_bkg = true;
    int box_padding_ud = 0;
    int box_padding_lr = 0;
    std::optional<Style> box_outline_style = std::nullopt;
    OutlineType outline_type = OutlineType::Line;
  };
  
  struct TextBoxDrawingArgsPos
  {
    TextBoxDrawingArgs base;
    RC pos;
  };
  
  struct TextBoxDrawingArgsAlign
  {
    TextBoxDrawingArgs base;
    VerticalAlignment v_align = VerticalAlignment::CENTER;
    HorizontalAlignment h_align = HorizontalAlignment::CENTER;
    int v_align_offs = 0;
    int h_align_offs = 0;
    bool framed_mode = true;
  };
  
  // ////
  
  enum class ButtonFrame { None, Space, SquareBrackets, AngleBrackets, Braces, Parentheses, Pipes };
  enum class TextFieldMode { Numeric, AlphaNumeric, Alphabetic, Hex, PrintableAscii, All };
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
  // __          ___     _            _
  // \ \        / (_)   | |          | |
  //  \ \  /\  / / _  __| | __ _  ___| |_
  //   \ \/  \/ / | |/ _` |/ _` |/ _ \ __|
  //    \  /\  /  | | (_| | (_| |  __/ |_
  //     \/  \/   |_|\__,_|\__, |\___|\__|
  //                        __/ |
  //                       |___/
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
  
  // /////////////////////////////////////////////////////////////
  //  ____        _   _
  // |  _ \      | | | |
  // | |_) |_   _| |_| |_ ___  _ __
  // |  _ <| | | | __| __/ _ \| '_ \
  // | |_) | |_| | |_| || (_) | | | |
  // |____/ \__,_|\__|\__\___/|_| |_|
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
  
  // /////////////////////////////////////////////////////////////
  //  ____        _   _               _____
  // |  _ \      | | | |             / ____|
  // | |_) |_   _| |_| |_ ___  _ __ | |  __ _ __ ___  _   _ _ __
  // |  _ <| | | | __| __/ _ \| '_ \| | |_ | '__/ _ \| | | | '_ \
  // | |_) | |_| | |_| || (_) | | | | |__| | | | (_) | |_| | |_) |
  // |____/ \__,_|\__|\__\___/|_| |_|\_____|_|  \___/ \__,_| .__/
  //                                                       | |
  //                                                       |_|
  // /////////////////////////////////////////////////////////////
  
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
  
  // /////////////////////////////////////////////////////////////
  //  _           _          _
  // | |         | |        | |
  // | |     __ _| |__   ___| |
  // | |    / _` | '_ \ / _ \ |
  // | |___| (_| | |_) |  __/ |
  // |______\__,_|_.__/ \___|_|
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
  
  // /////////////////////////////////////////////////////////////
  //  _______        _   ______ _      _     _
  // |__   __|      | | |  ____(_)    | |   | |
  //    | | _____  _| |_| |__   _  ___| | __| |
  //    | |/ _ \ \/ / __|  __| | |/ _ \ |/ _` |
  //    | |  __/>  <| |_| |    | |  __/ | (_| |
  //    |_|\___/_/\_\\__|_|    |_|\___|_|\__,_|
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
  
  // /////////////////////////////////////////////////////////////
  //   _____ _             _     _____ _      _
  //  / ____| |           | |   |  __ (_)    | |
  // | |  __| |_   _ _ __ | |__ | |__) |  ___| | _____ _ __
  // | | |_ | | | | | '_ \| '_ \|  ___/ |/ __| |/ / _ \ '__|
  // | |__| | | |_| | |_) | | | | |   | | (__|   <  __/ |
  //  \_____|_|\__, | .__/|_| |_|_|   |_|\___|_|\_\___|_|
  //            __/ | |
  //           |___/|_|
  // /////////////////////////////////////////////////////////////
  
  class GlyphPicker : public Widget
  {
    // cp_ : codepoint
    // fb_ : fallback
    // cg_ : current glyph
    // rg_ : recent glyphs
    PromptStyle prmpt_style;
    Style lbl_style;
    Style brck_style;
    Label rg_lbl;
    Label cg_lbl;
    Label cp_lbl;
    Label fb_lbl;
    Label hex_prefix_lbl;
    TextField cp_field; // preferred (UTF-8)
    TextField fb_field; // fallback (ASCII)
    std::string cp_str_prev;
    std::string fb_str_prev;
    
    std::array<t8::Glyph, 4> recent_glyphs;
    int sel_recent_idx = -1; // Current selection in newest-first display order.
    int recent_head = 0; // Next storage slot to write in the ring buffer.
    int recent_count = 0; // How many entries have been added so far.
    
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
        cp_field.clear();
      else
        cp_field.set_input(str::int2hex(g.preferred));
      
      if (g.fallback == t8::Glyph::none)
        fb_field.clear();
      else
        fb_field.set_input(std::string(1, g.fallback));
      
      cp_str_prev = cp_field.get_input();
      fb_str_prev = fb_field.get_input();
    }
    
  public:
    GlyphPicker(PromptStyle tf_style, Style label_style, Style hex_prefix_style, Style bracket_style,
                int tab, char clear_ch = '_', bool sel = false)
      : Widget(tab, sel)
      , prmpt_style(tf_style)
      , lbl_style(label_style)
      , brck_style(bracket_style)
      , rg_lbl("Recent Glyphs:", label_style)
      , cg_lbl("Current Glyph:", label_style)
      , cp_lbl("Preferred (Unicode):", label_style)
      , fb_lbl("Fallback (ASCII):", label_style)
      , hex_prefix_lbl("0x", hex_prefix_style)
      , cp_field(6, TextFieldMode::Hex, tf_style,
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
        
      if (cp_field.is_selected())
      {
        cp_field.update(curr_key, curr_special_key);
        if (cp_field.empty())
          current_glyph.preferred = t8::Glyph::none32;
        else if (cp_str_prev != cp_field.get_input())
          current_glyph.preferred = str::hex2int(cp_field.get_input());
        cp_str_prev = cp_field.get_input();
      }
      if (fb_field.is_selected())
      {
        fb_field.update(curr_key, curr_special_key);
        if (fb_field.empty())
          current_glyph.fallback = t8::Glyph::none;
        else if (fb_str_prev != fb_field.get_input())
          current_glyph.fallback = fb_field.get_input()[0];
        fb_str_prev = fb_field.get_input();
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
            glyph_style.bg_color = prmpt_style.bg_color_cursor;
            bracket_style.bg_color = prmpt_style.bg_color_cursor;
          }
          auto ss_vec = rg.format_short<CharT>(true, glyph_style, glyph_style, bracket_style);
          rg_col_offs = sh.write_buffer(ss_vec, pos + RC { 1, rg_col_start + rg_col_offs });
        }
      }
      
      cp_lbl.draw(sh, pos + RC { 2, 0 });
      hex_prefix_lbl.draw(sh, pos + RC { 2, cp_lbl.width() + 1 });
      cp_field.draw(sh, pos + RC { 2, cp_lbl.width() + 1 + hex_prefix_lbl.width() }, anim_ctr);
      
      fb_lbl.draw(sh, pos + RC { 3, 0 });
      fb_field.draw(sh, pos + RC { 3, cp_lbl.width() + 1 + hex_prefix_lbl.width() }, anim_ctr);
    }
    
    virtual int width() const override
    {
      return cp_lbl.width() + 1 + hex_prefix_lbl.width() + cp_field.width();
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
      cp_field.clear();
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
    
    virtual int num_components() const override { return 3; }
    
    virtual void set_component_focus(int sub_tab, bool selected) override
    {
      if (sub_tab == 0)
        sel_recent_idx = selected && recent_count > 0 ? std::max(0, sel_recent_idx) : -1;
      else if (sub_tab == 1)
        cp_field.set_selected(selected);
      else if (sub_tab == 2)
        fb_field.set_selected(selected);
    }
    
    void push_recent()
    {
      const auto glyph = get_canonicalized_glyph();
      if (glyph.fully_empty())
        return;
      if (!glyph.valid())
        return;
      
      int existing_idx = -1;
      for (int recent_idx = 0; recent_idx < recent_count; ++recent_idx)
      {
        if (recent_glyphs[recent_idx] == glyph)
        {
          existing_idx = recent_idx;
          break;
        }
      }
      
      const int move_count = existing_idx == -1
        ? std::min(recent_count, stlutils::sizeI(recent_glyphs) - 1)
        : existing_idx;
      
      // Move older glyphs towards the back.
      for (int recent_idx = move_count; recent_idx > 0; --recent_idx)
        recent_glyphs[recent_idx] = recent_glyphs[recent_idx - 1];
      
      recent_glyphs[0] = glyph;
      
      if (existing_idx == -1)
      recent_count = std::min(recent_count + 1, stlutils::sizeI(recent_glyphs));
      
      if (sel_recent_idx != -1)
        sel_recent_idx = 0;
    }
  };
  
  // /////////////////////////////////////////////////////////////
  //   _____      _            _____ _      _
  //  / ____|    | |          |  __ (_)    | |
  // | |     ___ | | ___  _ __| |__) |  ___| | _____ _ __
  // | |    / _ \| |/ _ \| '__|  ___/ |/ __| |/ / _ \ '__|
  // | |___| (_) | | (_) | |  | |   | | (__|   <  __/ |
  //  \_____\___/|_|\___/|_|  |_|   |_|\___|_|\_\___|_|
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
  
  // /////////////////////////////////////////////////////////////
  //  _______        _   ____
  // |__   __|      | | |  _ \
  //    | | _____  _| |_| |_) | _____  __
  //    | |/ _ \ \/ / __|  _ < / _ \ \/ /
  //    | |  __/>  <| |_| |_) | (_) >  <
  //    |_|\___/_/\_\\__|____/ \___/_/\_\
  // /////////////////////////////////////////////////////////////

  // #NOTE: Supported template argument types: std::string and t8::GlyphString.
  template<typename StrT = std::string>
  class TextBox
  {
    bool allow_panel_size_caching = false;
  
  protected:
    str::StringBox<StrT> sb;
    size_t N = 0;
    size_t len_max = 0;
    std::vector<Style> line_styles;
    std::vector<std::pair<RC, Style>> override_textel_styles;
    std::optional<RC> cached_panel_size = std::nullopt;
    str::Adjustment master_adjustment = str::Adjustment::Left;
    
    void init()
    {
      N = sb.size();
      invalidate_layout();
    }
    
    virtual bool has_buttons() const { return false; }
    
    template<int NR, int NC>
    TextBoxDrawingArgsPos get_drawing_args_pos(const TextBoxDrawingArgsAlign& args)
    {
      int box_padding_ud = args.base.box_padding_ud;
      int box_padding_lr = args.base.box_padding_lr;
      
      const auto panel_size = fetch_panel_size();
      int panel_height = panel_size.r;
      int panel_width = panel_size.c;
    
      RC pos { 0, 0 };
      
      auto r_diff = std::max(0, NR - panel_height);
      auto c_diff = std::max(0, NC - panel_width);
      auto mid_v = static_cast<int>(std::round(r_diff*0.5f) - (r_diff%2 == 1)*0.5f);
      auto mid_h = static_cast<int>(std::round(c_diff*0.5f) - (c_diff%2 == 1)*0.5f);
      
      switch (args.v_align)
      {
        case VerticalAlignment::TOP:
          pos.r = 2 + box_padding_ud - !args.framed_mode;
          break;
        case VerticalAlignment::CENTER:
          pos.r = mid_v;
          break;
        case VerticalAlignment::BOTTOM:
          pos.r = static_cast<int>(NR - N) - box_padding_ud - 2 + !args.framed_mode; // FIXED!
          break;
      }
      
      switch (args.h_align)
      {
        case HorizontalAlignment::LEFT:
          pos.c = 2 + box_padding_lr - !args.framed_mode;
          break;
        case HorizontalAlignment::CENTER:
          pos.c = mid_h;
          break;
        case HorizontalAlignment::RIGHT:
          pos.c = static_cast<int>(NC - len_max) - box_padding_lr - 2 + !args.framed_mode; // FIXED!
          break;
      }
      
      pos.r += args.v_align_offs;
      pos.c += args.h_align_offs;
      
      TextBoxDrawingArgsPos pargs;
      pargs.pos = pos;
      pargs.base = args.base;
      
      return pargs;
    }
    
    virtual RC measure_panel_size() const
    {
      int r_len = static_cast<int>(TextBox<StrT>::N);
      int c_len = static_cast<int>(TextBox<StrT>::len_max);
      return { r_len, c_len };
    }
    
    RC fetch_panel_size()
    {
      if (!cached_panel_size.has_value() || !allow_panel_size_caching)
      {
        N = sb.size();
        len_max = 0;
        for (size_t l_idx = 0; l_idx < N; ++l_idx)
          math::maximize(len_max, sb[l_idx].size());
        
        cached_panel_size = measure_panel_size();
        
        for (size_t l_idx = 0; l_idx < N; ++l_idx)
          sb[l_idx] = str::adjust_str(sb[l_idx], master_adjustment, static_cast<int>(len_max));
      }
      return cached_panel_size.value();
    }
    
    virtual void on_pre_draw() {}
    
  public:
    virtual ~TextBox() = default;
    TextBox() = default;
    TextBox(str::Adjustment master_adj, bool cache_panel_size)
      : allow_panel_size_caching(cache_panel_size)
      , master_adjustment(master_adj)
    {}
    TextBox(size_t num_lines, str::Adjustment master_adj = str::Adjustment::Left)
      : sb(num_lines)
      , master_adjustment(master_adj)
    {
      init();
    }
    TextBox(const std::vector<StrT>& text_lines, const std::vector<Style>& styles = {},
            str::Adjustment master_adj = str::Adjustment::Left)
      : sb(text_lines)
      , line_styles(styles)
      , master_adjustment(master_adj)
    {
      init();
      if (text_lines.size() != line_styles.size())
        line_styles.clear();
    }
    TextBox(const StrT& text, str::Adjustment master_adj = str::Adjustment::Left)
      : sb(text)
      , master_adjustment(master_adj)
    {
      init();
    }
        
    bool empty() const noexcept
    {
      return sb.empty();
    }
    
    void set_text(const std::vector<StrT>& text_lines,
                  const std::vector<Style>& styles = {},
                  const std::vector<std::pair<RC, Style>>& override_styles = {},
                  str::Adjustment master_adj = str::Adjustment::Left)
    {
      sb = str::StringBox<StrT> { text_lines };
      line_styles = styles;
      override_textel_styles = override_styles;
      master_adjustment = master_adj;
      init();
      if (text_lines.size() != line_styles.size())
        line_styles.clear();
    }
    
    void set_text(const StrT& text,
                  str::Adjustment master_adj = str::Adjustment::Left)
    {
      sb = str::StringBox<StrT> { text };
      override_textel_styles.clear();
      master_adjustment = master_adj;
      init();
    }
    
    void set_text(const StrT& text, Style style, str::Adjustment master_adj = str::Adjustment::Left)
    {
      sb = str::StringBox<StrT> { text };
      line_styles.clear();
      line_styles.emplace_back(style);
      override_textel_styles.clear();
      master_adjustment = master_adj;
      init();
    }
    
    void set_num_lines(size_t num_lines)
    {
      sb = str::StringBox<StrT>(num_lines);
      init();
    }
    
    StrT& operator[](size_t r_idx)
    {
      return sb[r_idx];
      invalidate_layout();
    }
    const StrT& operator[](size_t r_idx) const
    {
      return sb[r_idx];
    }
    
    template<int NR, int NC, typename CharT>
    void draw(ScreenHandler<NR, NC, CharT>& sh, const TextBoxDrawingArgsPos& args)
    {
      on_pre_draw();
    
      const auto& pos = args.pos;
      auto panel_size = fetch_panel_size();
      int panel_height = panel_size.r;
      int panel_width = panel_size.c;
      const Style& box_style = args.base.box_style;
      bool do_draw_box_outline = args.base.draw_box_outline;
      bool do_draw_box_bkg = args.base.draw_box_bkg;
      int box_padding_ud = args.base.box_padding_ud;
      int box_padding_lr = args.base.box_padding_lr;
#ifdef __linux__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
#endif
      std::optional<Style> box_outline_style = args.base.box_outline_style;
#ifdef __linux__
#pragma GCC diagnostic pop
#endif
      OutlineType outline_type = args.base.outline_type;
      
      for (const auto& rc_style : override_textel_styles)
        if (rc_style.first.r < static_cast<int>(N) && rc_style.first.c < static_cast<int>(len_max))
          sh.write_buffer(sb[rc_style.first.r][rc_style.first.c],
                          pos.r + rc_style.first.r, pos.c + rc_style.first.c,
                          rc_style.second);
      for (size_t l_idx = 0; l_idx < N; ++l_idx)
        sh.write_buffer(sb[l_idx], pos.r + static_cast<int>(l_idx), pos.c, line_styles.empty() ? box_style : line_styles[l_idx]);
      
      int r = pos.r - 1 - box_padding_ud;
      int c = pos.c - 1 - box_padding_lr;
      int r_len = panel_height + static_cast<int>(2 + 2*has_buttons() + 2*box_padding_ud);
      int c_len = panel_width + static_cast<int>(2 + 2*box_padding_lr);
      if (do_draw_box_outline)
        draw_box_outline(sh, r, c, r_len, c_len, outline_type, box_outline_style.value_or(box_style));
      if (do_draw_box_bkg)
        draw_box(sh, r, c, r_len, c_len, box_style);
    }
    
    template<int NR, int NC, typename CharT>
    void draw(ScreenHandler<NR, NC, CharT>& sh, const TextBoxDrawingArgsAlign& args)
    {
      auto pargs = get_drawing_args_pos<NR, NC>(args);
      
      draw(sh, pargs);
    }
    
    void invalidate_layout()
    {
      cached_panel_size = std::nullopt;
    }
  };
  
  // /////////////////////////////////////////////////////////////
  //  _____  _       _
  // |  __ \(_)     | |
  // | |  | |_  __ _| | ___   __ _
  // | |  | | |/ _` | |/ _ \ / _` |
  // | |__| | | (_| | | (_) | (_| |
  // |_____/|_|\__,_|_|\___/ \__, |
  //                          __/ |
  //                         |___/
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
    
    bool text_field_empty(int tab)
    {
      auto it = stlutils::find_if(text_fields, [tab](const auto& tfp) { return tfp.second->get_tab_order() == tab; });
      if (it != text_fields.end())
        return it->second->empty();
      return true; // Treat as empty if unable to find matching text field.
    }
    
    GlyphPicker& emplace_glyph_picker(const RC& pos,
                                      PromptStyle tf_style, Style label_style, Style hex_prefix_style, Style bracket_style,
                                      int tab, char clear_ch = '_', bool sel = false)
    {
      auto uptr = std::make_unique<GlyphPicker>(tf_style, label_style, hex_prefix_style, bracket_style, tab, clear_ch, sel);
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
        return short_format ? it->second->get_disp_sstr_short() : it->second->get_disp_sstr_long();
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
  
  // /////////////////////////////////////////////////////////////
  //  _____                          ____
  // |  __ \                        |  _ \
  // | |__) |_ _ _ __ __ _ _ __ ___ | |_) | __ _ ___  ___
  // |  ___/ _` | '__/ _` | '_ ` _ \|  _ < / _` / __|/ _ \
  // | |  | (_| | | | (_| | | | | | | |_) | (_| \__ \  __/
  // |_|   \__,_|_|  \__,_|_| |_| |_|____/ \__,_|___/\___|
  // /////////////////////////////////////////////////////////////
  
  struct ParamBase
  {
    std::string name;
  
    ParamBase(const std::string& a_name) : name(a_name) {}
    virtual ~ParamBase() = default;
    virtual std::string str() const = 0;
  };
  
  // /////////////////////////////////////////////////////////////
  //  _____       __ _____
  // |  __ \     / _|  __ \
  // | |__) |___| |_| |__) |_ _ _ __ __ _ _ __ ___
  // |  _  // _ \  _|  ___/ _` | '__/ _` | '_ ` _ \
  // | | \ \  __/ | | |  | (_| | | | (_| | | | | | |
  // |_|  \_\___|_| |_|   \__,_|_|  \__,_|_| |_| |_|
  // /////////////////////////////////////////////////////////////
  
  template<typename T>
  struct RefParam : ParamBase
  {
    T* var = nullptr;
    T step = static_cast<T>(0);
    T min = static_cast<T>(0);
    T max = static_cast<T>(0);
    
    RefParam(const std::string& a_name, T* a_var, T a_step, T a_min, T a_max)
      : ParamBase(a_name)
      , var(a_var)
      , step(a_step)
      , min(a_min)
      , max(a_max)
    {}
    
    virtual std::string str() const override
    {
      return name + ": " + (var == nullptr ? "null" : std::to_string(*var));
    }
  };
  
  // /////////////////////////////////////////////////////////////
  //  _____
  // |  __ \
  // | |__) |_ _ _ __ __ _ _ __ ___
  // |  ___/ _` | '__/ _` | '_ ` _ \
  // | |  | (_| | | | (_| | | | | | |
  // |_|   \__,_|_|  \__,_|_| |_| |_|
  // /////////////////////////////////////////////////////////////
  
  template<typename T>
  class Param : public RefParam<T>
  {
    T instance;
    
  public:
    Param(const std::string& a_name, T* a_var, T a_step, T a_min, T a_max)
      : RefParam<T>(a_name, a_var, a_step, a_min, a_max)
    {
      instance = *a_var;
      RefParam<T>::var = &instance;
    }
  };
  
  // /////////////////////////////////////////////////////////////
  //  _______        _   ____            _____       _
  // |__   __|      | | |  _ \          |  __ \     | |
  //    | | _____  _| |_| |_) | _____  _| |  | | ___| |__  _   _  __ _
  //    | |/ _ \ \/ / __|  _ < / _ \ \/ / |  | |/ _ \ '_ \| | | |/ _` |
  //    | |  __/>  <| |_| |_) | (_) >  <| |__| |  __/ |_) | |_| | (_| |
  //    |_|\___/_/\_\\__|____/ \___/_/\_\_____/ \___|_.__/ \__,_|\__, |
  //                                                              __/ |
  //                                                             |___/
  // /////////////////////////////////////////////////////////////
  
  class TextBoxDebug : public TextBox<std::string>
  {
    std::vector<std::unique_ptr<ParamBase>> params;
    
    void init()
    {
      TextBox<std::string>::sb.text_lines.resize(params.size());
      TextBox<std::string>::init();
    }
    
  protected:
    virtual void on_pre_draw() override
    {
      for (size_t p_idx = 0; p_idx < TextBox<std::string>::N; ++p_idx)
        TextBox<std::string>::sb[p_idx] = params[p_idx]->str();
    }
    
  public:
    TextBoxDebug()
      : TextBox<std::string>(str::Adjustment::Left, false)
    {}
    
    TextBoxDebug(str::Adjustment master_adj)
      : TextBox<std::string>(master_adj, false)
    {}
  
    template<typename T>
    void add(const RefParam<T>& ref_param)
    {
      params.emplace_back(std::make_unique<RefParam<T>>(ref_param));
      init();
    }
    
    template<typename T>
    void add(const std::string& name, T* var_ptr, T step = static_cast<T>(0), T min = static_cast<T>(0), T max = static_cast<T>(0))
    {
      params.emplace_back(std::make_unique<RefParam<T>>(name, var_ptr, step, min, max));
      init();
    }
    
    // Usage: tbd->ref_tmp(PARAM(var)) = var;
    template<typename T>
    T& ref_tmp(const std::string& name, T* var_ptr)
    {
      auto it = stlutils::find_if(params, [&name](auto& p) { return p->name == name; });
      if (it == params.end())
      {
        auto& ref = params.emplace_back(std::make_unique<Param<T>>(name, var_ptr, static_cast<T>(0), static_cast<T>(*var_ptr), static_cast<T>(*var_ptr)));
        init();
        auto* tmp_param = dynamic_cast<Param<T>*>(ref.get());
        return *tmp_param->var;
      }
      auto* tmp_param = dynamic_cast<Param<T>*>(it->get());
      return *tmp_param->var;
    }
    
    template<typename T>
    T& ref(const std::string& name, T* var_ptr)
    {
      auto it = stlutils::find_if(params, [&name](auto& p) { return p->name == name; });
      if (it == params.end())
      {
        auto& ref = params.emplace_back(std::make_unique<RefParam<T>>(name, var_ptr, static_cast<T>(0), static_cast<T>(*var_ptr), static_cast<T>(*var_ptr)));
        init();
        auto* param = dynamic_cast<RefParam<T>*>(ref.get());
        return *param->var;
      }
      auto* param = dynamic_cast<RefParam<T>*>(it->get());
      return *param->var;
    }
    
    bool remove(const std::string& name)
    {
      return stlutils::erase_if(params, [&name](auto& p) { return p->name == name; });
    }
    
    void clear()
    {
      params.clear();
      TextBox<std::string>::sb.text_lines.clear();
      init();
    }
    
    virtual RC measure_panel_size() const override
    {
      auto panel_size = TextBox<std::string>::measure_panel_size();
      
      // Row dimension is already set via TextBoxDebug::init().
      
      for (size_t p_idx = 0; p_idx < TextBox<std::string>::N; ++p_idx)
        math::maximize(panel_size.c, str::lenI(params[p_idx]->str()));
      
      return panel_size;
    }
  };

}

