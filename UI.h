//
//  UI.h
//  Termin8or
//
//  Created by Rasmus Anthin on 2024-07-08.
//

#pragma once
#include "RC.h"
#include "KeyboardEnums.h"
#include <Core/StringBox.h>
#include <Core/Utils.h>

#define PARAM(var) #var, &var

namespace ui
{

  enum class VerticalAlignment { TOP, CENTER, BOTTOM };
  enum class HorizontalAlignment { LEFT, CENTER, RIGHT };
  
  struct TextBoxDrawingArgs
  {
    styles::Style box_style;
    bool draw_box_outline = true;
    bool draw_box_bkg = true;
    int box_padding_ud = 0;
    int box_padding_lr = 0;
    std::optional<styles::Style> box_outline_style = std::nullopt;
    drawing::OutlineType outline_type = drawing::OutlineType::Line;
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
  enum class TextFieldMode { Numeric, AlphaNumeric, Alphabetic, All };
  enum class ColorPickerCursorColoring { BlackWhite, Contrast };
  
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
  
    int get_tab_order() const { return tab_order; }
    bool try_tab_select(int tab)
    {
      if (tab_order == tab)
      {
        selected = true;
        return true;
      }
      return false;
    }
    
    void set_selected(bool sel) { selected = sel; }
    void toggle_selected() { math::toggle(selected); }
    bool is_selected() const { return selected; }
  };
  
  class Button : public Widget
  {
    std::string text;
    styles::ButtonStyle style;
    ButtonFrame frame = ButtonFrame::SquareBrackets;
    
  public:
    Button(const std::string& txt, styles::ButtonStyle btn_style, ButtonFrame btn_frame, int tab = 0, bool sel = false)
      : Widget(tab, sel)
      , text(txt)
      , style(btn_style)
      , frame(btn_frame)
    {}
    
    int get_width() const
    {
      int left_right = 2;
      if (frame == ButtonFrame::None)
        left_right = 0;
      return left_right + static_cast<int>(text.size());
    }
    
    template<int NR, int NC>
    void draw(ScreenHandler<NR, NC>& sh, const RC& pos) const
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
  
  class ButtonGroup
  {
    std::vector<Button> buttons;
    int selected = -1;
    
  public:
    bool empty() const { return buttons.empty(); }
    int size() const { return stlutils::sizeI(buttons); }
    
    void clear_selections()
    {
      for (auto& btn : buttons)
        btn.set_selected(false);
    }
    
    bool try_tab_select(int tab)
    {
      for (int b_idx = 0; b_idx < stlutils::sizeI(buttons); ++b_idx)
      {
        auto& btn = buttons[b_idx];
        if (btn.get_tab_order() == tab)
        {
          btn.set_selected(true);
          selected = b_idx;
          return true;
        }
      }
      return false;
    }
  
    void add(const Button& btn)
    {
      buttons.emplace_back(btn);
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
        return &buttons[selected];
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
    
    template<int NR, int NC>
    void draw(ScreenHandler<NR, NC>& sh, const RC& pos, int width) const
    {
      if (buttons.empty())
        return;
    
      int width_btns = stlutils::sum<int>(buttons, [](const auto& btn) { return btn.get_width(); });
      int spacing = (width - width_btns) / static_cast<int>(buttons.size() - (buttons.size() > 1));
      
      int c = pos.c;
      for (const auto& btn : buttons)
      {
        btn.draw(sh, RC { pos.r, c });
        c += btn.get_width() + spacing;
      }
    }
  };
  
  class TextField : public Widget
  {
    int field_width = 0;
    TextFieldMode mode = TextFieldMode::AlphaNumeric;
    styles::PromptStyle style;
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
    TextField(int width, TextFieldMode tf_mode, styles::PromptStyle tf_style, int tab = 0, char clear_ch = '_', bool sel = false)
      : Widget(tab, sel)
      , field_width(width)
      , mode(tf_mode)
      , style(tf_style)
      , clear_char(clear_ch)
    {
      input = str::rep_char(clear_ch, width);
    }
  
    void update(char curr_key, keyboard::SpecialKey curr_special_key)
    {
      if (!is_selected())
        return;
      if (mode == TextFieldMode::All && curr_key != 0)
        add_char(curr_key);
      if (str::is_digit(curr_key))
      {
        if (mode == TextFieldMode::Numeric || mode == TextFieldMode::AlphaNumeric)
          add_char(curr_key);
      }
      else if (str::is_letter(curr_key))
      {
        if (mode == TextFieldMode::Alphabetic || mode == TextFieldMode::AlphaNumeric)
          add_char(curr_key);
      }
      else if (curr_special_key == keyboard::SpecialKey::Backspace)
        backspace();
    }
    
    template<int NR, int NC>
    void draw(ScreenHandler<NR, NC>& sh, const RC& pos, int anim_ctr) const
    {
      if (is_selected())
      {
        auto bg_color_caret = (anim_ctr == 0) ? style.bg_color_cursor : style.bg_color;
        sh.write_buffer(std::string(1, input[get_caret_pos()]), pos.r, pos.c + get_caret_pos(), style.fg_color, bg_color_caret);
      }
    
      sh.write_buffer(input, pos, style.fg_color, style.bg_color);
    }
    
    const std::string get_input() const
    {
      return input.substr(0, caret);
    }
    
    void set_input(const std::string& str)
    {
      int len = std::min(input.length(), str.length());
      input.replace(0, len, str.substr(0, len));
      caret = len;
    }
    
    void clear_input()
    {
      input = str::rep_char(clear_char, field_width);
      caret = 0;
    }
    
    bool empty() const
    {
      return input == str::rep_char(clear_char, field_width);
    }
  };
  
  class ColorPicker : public Widget
  {
    Color fg_color_sel;
    Color fg_color_sel_hilite;
    ColorPickerCursorColoring fg_cursor_coloring;
    char select_char;
    char unselect_char = ' ';
    std::string field;
    int caret = 0;
    static constexpr int c_num_colors = 16; // 16
    // #FIXME: Add transparency (T2) support.
    
    Color get_color(int idx) const
    {
      if (0 <= idx && idx < 16)
        return static_cast<Color>(idx + 1);
      return Color::Default;
    }
    
  public:
    ColorPicker(Color fg_sel, Color fg_sel_hilite, ColorPickerCursorColoring fg_cursor_coloring_scheme, int tab = 0, char sel_char = '*', char unsel_char = ' ', bool sel = false)
      : Widget(tab, sel)
      , fg_color_sel(fg_sel)
      , fg_color_sel_hilite(fg_sel_hilite)
      , fg_cursor_coloring(fg_cursor_coloring_scheme)
      , select_char(sel_char)
      , unselect_char(unsel_char)
    {
      field = str::rep_char(unselect_char, c_num_colors);
    }
    
    void update(keyboard::SpecialKey curr_special_key)
    {
      if (!is_selected())
        return;
      
      if (curr_special_key == keyboard::SpecialKey::Left)
      {
        caret--;
        if (caret < 0)
          caret = 0;
      }
      else if (curr_special_key == keyboard::SpecialKey::Right)
      {
        caret++;
        if (caret >= c_num_colors)
          caret = c_num_colors - 1;
      }
    }
    
    template<int NR, int NC>
    void draw(ScreenHandler<NR, NC>& sh, const RC& pos, int anim_ctr) const
    {
      auto caret_fg_color = anim_ctr == 0 ? fg_color_sel : fg_color_sel_hilite;
      auto caret_bg_color = get_color(caret);
      if (!is_selected())
      {
        switch (fg_cursor_coloring)
        {
          case ColorPickerCursorColoring::BlackWhite:
            caret_fg_color = color::is_dark(caret_bg_color, true).value_or(false) ? Color::White : Color::Black;
            break;
          case ColorPickerCursorColoring::Contrast:
            caret_fg_color = color::get_contrast_color(caret_bg_color);
            break;
        }
      }
      sh.write_buffer(std::string(1, select_char), pos.r, pos.c + caret, caret_fg_color, caret_bg_color);
    
      for (int col = 0; col < c_num_colors; ++col)
        sh.write_buffer(std::string(1, unselect_char), pos.r, pos.c + col, Color::Transparent2, get_color(col));
    }
    
    Color get_color() const
    {
      return get_color(caret);
    }
    
    void set_color(Color color)
    {
      int idx = static_cast<int>(color) - 1;
      if (idx < 0)
        return;
      caret = idx;
    }
    
    void clear()
    {
      caret = 0;
    }
  };

  class TextBox
  {
  protected:
    str::StringBox sb;
    size_t N = 0;
    size_t len_max = 0;
    std::vector<styles::Style> line_styles;
    std::vector<std::pair<RC, styles::Style>> override_textel_styles;
    
    void init()
    {
      N = sb.size();
    }
    
    virtual bool has_buttons() const { return false; }
    
    template<int NR, int NC>
    TextBoxDrawingArgsPos get_drawing_args_pos(const TextBoxDrawingArgsAlign& args)
    {
      int box_padding_ud = args.base.box_padding_ud;
      int box_padding_lr = args.base.box_padding_lr;
    
      RC pos { 0, 0 };
      
      auto mid_v = static_cast<int>(std::round((NR - N)*0.5f) - ((NR - N)%2 == 1)*0.5f);
      auto mid_h = static_cast<int>(std::round((NC - len_max)*0.5f) - ((NC - len_max)%2 == 1)*0.5f);
      
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
    
  public:
    virtual ~TextBox() = default;
    TextBox() = default;
    TextBox(size_t num_lines)
      : sb(num_lines)
    {
      init();
    }
    TextBox(const std::vector<std::string>& text_lines, const std::vector<styles::Style>& styles = {})
      : sb(text_lines)
      , line_styles(styles)
    {
      init();
      if (text_lines.size() != line_styles.size())
        line_styles.clear();
    }
    TextBox(const std::string& text)
      : sb(text)
    {
      init();
    }
    
    bool empty() const noexcept
    {
      return sb.empty();
    }
    
    void set_text(const std::vector<std::string>& text_lines,
                  const std::vector<styles::Style>& styles = {},
                  const std::vector<std::pair<RC, styles::Style>>& override_styles = {})
    {
      sb = str::StringBox { text_lines };
      line_styles = styles;
      override_textel_styles = override_styles;
      init();
      if (text_lines.size() != line_styles.size())
        line_styles.clear();
    }
    
    void set_text(const std::string& text)
    {
      sb = str::StringBox { text };
      override_textel_styles.clear();
      init();
    }
    
    void set_text(const std::string& text, styles::Style style)
    {
      sb = str::StringBox { text };
      line_styles.clear();
      line_styles.emplace_back(style);
      override_textel_styles.clear();
      init();
    }
    
    void set_num_lines(size_t num_lines)
    {
      sb = str::StringBox(num_lines);
      init();
    }
    
    std::string& operator[](size_t r_idx) { return sb[r_idx]; }
    
    virtual void calc_pre_draw(str::Adjustment adjustment)
    {
      len_max = 0;
      for (size_t l_idx = 0; l_idx < N; ++l_idx)
        math::maximize(len_max, sb[l_idx].size());
      for (size_t l_idx = 0; l_idx < N; ++l_idx)
        sb[l_idx] = str::adjust_str(sb[l_idx], adjustment, static_cast<int>(len_max));
    }
    
    template<int NR, int NC>
    void draw(ScreenHandler<NR, NC>& sh, const TextBoxDrawingArgsPos& args)
    {
      auto pos = args.pos;
      const styles::Style& box_style = args.base.box_style;
      bool draw_box_outline = args.base.draw_box_outline;
      bool draw_box_bkg = args.base.draw_box_bkg;
      int box_padding_ud = args.base.box_padding_ud;
      int box_padding_lr = args.base.box_padding_lr;
      std::optional<styles::Style> box_outline_style = args.base.box_outline_style;
      drawing::OutlineType outline_type = args.base.outline_type;
            
      for (const auto& rc_style : override_textel_styles)
        if (rc_style.first.r < N && rc_style.first.c < len_max)
          sh.write_buffer(std::string(1, sb[rc_style.first.r][rc_style.first.c]),
                          pos.r + rc_style.first.r, pos.c + rc_style.first.c,
                          rc_style.second);
      for (size_t l_idx = 0; l_idx < N; ++l_idx)
        sh.write_buffer(sb[l_idx], pos.r + static_cast<int>(l_idx), pos.c, line_styles.empty() ? box_style : line_styles[l_idx]);
      
      int r = pos.r - 1 - box_padding_ud;
      int c = pos.c - 1 - box_padding_lr;
      int r_len = static_cast<int>(N + 2 + 2*has_buttons() + 2*box_padding_ud);
      int c_len = static_cast<int>(len_max + 2 + 2*box_padding_lr);
      if (draw_box_outline)
        drawing::draw_box_outline(sh, r, c, r_len, c_len, outline_type, box_outline_style.value_or(box_style));
      if (draw_box_bkg)
        drawing::draw_box(sh, r, c, r_len, c_len, box_style);
    }
    
    template<int NR, int NC>
    void draw(ScreenHandler<NR, NC>& sh, const TextBoxDrawingArgsAlign& args)
    {
      auto pargs = get_drawing_args_pos<NR, NC>(args);
      
      draw(sh, pargs);
    }
  };
  
  class Dialog : public TextBox
  {
    std::vector<std::tuple<RC, styles::Style, char>> override_textels_pre;
    ButtonGroup button_group; // Buttons have their own reserved row two rows down.
    std::vector<std::pair<RC, TextField>> text_fields;
    std::vector<std::pair<RC, ColorPicker>> color_pickers;
    int tab_idx = 0;
    int max_tab_idx = 0;
    
    virtual bool has_buttons() const override { return !button_group.empty(); }
    
  public:
    Dialog() = default;
    Dialog(size_t num_lines)
      : TextBox(num_lines)
    {}
    Dialog(const std::vector<std::string>& text_lines, const std::vector<styles::Style>& styles = {})
      : TextBox(text_lines, styles)
    {}
    Dialog(const std::string& text)
      : TextBox(text)
    {}
    
    void set_tab_order(int tab)
    {
      // Clear widgets
      button_group.clear_selections();
      for (auto& tfp : text_fields)
        tfp.second.set_selected(false);
      for (auto& cpp : color_pickers)
        cpp.second.set_selected(false);
      
      // Select a widget
      if (button_group.try_tab_select(tab))
        return;
      for (auto& tfp : text_fields)
        if (tfp.second.try_tab_select(tab))
          return;
      for (auto& cpp : color_pickers)
        if (cpp.second.try_tab_select(tab))
          return;
    }
    
    void set_textel_pre(const RC& local_pos, char ch, Color fg_color, Color bg_color)
    {
      //stlutils::emplace_back_if_not(override_textels_pre,
      //  std::tuple<RC, styles::Style, char> { local_pos, { fg_color, bg_color }, ch},
      //  [&local_pos](const auto& otp) { return std::get<0>(otp) == local_pos; });
      auto it = stlutils::find_if(override_textels_pre,
                  [&local_pos](const auto& otp) { return std::get<0>(otp) == local_pos; });
      if (it != override_textels_pre.end())
      {
        std::get<1>(*it) = { fg_color, bg_color };
        std::get<2>(*it) = ch;
      }
      else
        override_textels_pre.emplace_back(local_pos, styles::Style { fg_color, bg_color }, ch);
    }
    
    void add_button(const Button& button)
    {
      button_group.add(button);
      math::maximize(max_tab_idx, button.get_tab_order());
    }
    
    void set_button_selection(int sel_idx, bool sel)
    {
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
    
    void add_text_field(const RC& pos, const TextField& tf)
    {
      text_fields.emplace_back(pos, tf);
      math::maximize(max_tab_idx, tf.get_tab_order());
    }
    
    const std::string get_text_field_input(int tab) const
    {
      auto it = stlutils::find_if(text_fields, [tab](const auto& tfp) { return tfp.second.get_tab_order() == tab; });
      if (it != text_fields.end())
        return it->second.get_input();
      return "";
    }
    
    void set_text_field_input(int tab, const std::string& str)
    {
      auto it = stlutils::find_if(text_fields, [tab](const auto& tfp) { return tfp.second.get_tab_order() == tab; });
      if (it != text_fields.end())
        return it->second.set_input(str);
    }
    
    void clear_text_field_input(int tab)
    {
      auto it = stlutils::find_if(text_fields, [tab](const auto& tfp) { return tfp.second.get_tab_order() == tab; });
      if (it != text_fields.end())
        return it->second.clear_input();
    }
    
    bool text_field_empty(int tab)
    {
      auto it = stlutils::find_if(text_fields, [tab](const auto& tfp) { return tfp.second.get_tab_order() == tab; });
      if (it != text_fields.end())
        return it->second.empty();
      return true; // Treat as empty if unable to find matching text field.
    }
    
    void add_color_picker(const RC& pos, const ColorPicker& cp)
    {
      color_pickers.emplace_back(pos, cp);
      math::maximize(max_tab_idx, cp.get_tab_order());
    }
    
    const Color get_color_picker_color(int tab) const
    {
      auto it = stlutils::find_if(color_pickers, [tab](const auto& cpp) { return cpp.second.get_tab_order() == tab; });
      if (it != color_pickers.end())
        return it->second.get_color();
      return Color::Default;
    }
    
    void set_color_picker_color(int tab, Color color)
    {
      auto it = stlutils::find_if(color_pickers, [tab](const auto& cpp) { return cpp.second.get_tab_order() == tab; });
      if (it != color_pickers.end())
        return it->second.set_color(color);
    }
    
    void reset_color_picker(int tab)
    {
      auto it = stlutils::find_if(color_pickers, [tab](const auto& cpp) { return cpp.second.get_tab_order() == tab; });
      if (it != color_pickers.end())
        return it->second.clear();
    }
    
    void update(char curr_key, keyboard::SpecialKey curr_special_key)
    {
      if (curr_special_key == keyboard::SpecialKey::Tab)
      {
        tab_idx = (tab_idx + 1) % (max_tab_idx + 1);
        set_tab_order(tab_idx);
      }
      else if (curr_special_key == keyboard::SpecialKey::Left)
        dec_button_selection();
      else if (curr_special_key == keyboard::SpecialKey::Right)
        inc_button_selection();

      for (auto& tfp : text_fields)
        tfp.second.update(curr_key, curr_special_key);
      for (auto& cpp : color_pickers)
        cpp.second.update(curr_special_key);
    }
    
    template<int NR, int NC>
    void draw(ScreenHandler<NR, NC>& sh, const TextBoxDrawingArgsPos& args, int anim_ctr)
    {
      auto pos = args.pos;
      
      for (const auto& ot : override_textels_pre)
      {
        const auto& tp = std::get<0>(ot);
        if (math::in_range<int>(tp.r, 0, static_cast<int>(N), Range::ClosedOpen) &&
            math::in_range<int>(tp.c, 0, static_cast<int>(len_max), Range::ClosedOpen))
        {
          sh.write_buffer(std::string(1, std::get<2>(ot)),
                          pos.r + tp.r, pos.c + tp.c,
                          std::get<1>(ot));
        }
      }
                  
      button_group.draw(sh, { pos.r + static_cast<int>(N) + 1, pos.c }, static_cast<int>(len_max));
      
      for (const auto& tfp : text_fields)
        tfp.second.draw(sh, pos + tfp.first, anim_ctr);
        
      for (const auto& cpp : color_pickers)
        cpp.second.draw(sh, pos + cpp.first, anim_ctr);
        
      TextBox::draw(sh, args);
    }
    
    template<int NR, int NC>
    void draw(ScreenHandler<NR, NC>& sh, const TextBoxDrawingArgsAlign& args, int anim_ctr)
    {
      auto pargs = get_drawing_args_pos<NR, NC>(args);
      
      draw(sh, pargs, anim_ctr);
    }
  };
  
  struct ParamBase
  {
    std::string name;
  
    ParamBase(const std::string& a_name) : name(a_name) {}
    virtual ~ParamBase() = default;
    virtual std::string str() const = 0;
  };
  
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
  
  class TextBoxDebug : public TextBox
  {
    std::vector<std::unique_ptr<ParamBase>> params;
    
  public:
    TextBoxDebug() = default;
    
    template<typename T>
    void add(const RefParam<T>& ref_param)
    {
      params.emplace_back(std::make_unique<RefParam<T>>(ref_param));
      sb.text_lines.resize(params.size());
      init();
    }
    
    template<typename T>
    void add(const std::string& name, T* var_ptr, T step = static_cast<T>(0), T min = static_cast<T>(0), T max = static_cast<T>(0))
    {
      params.emplace_back(std::make_unique<RefParam<T>>(name, var_ptr, step, min, max));
      sb.text_lines.resize(params.size());
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
        sb.text_lines.resize(params.size());
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
        sb.text_lines.resize(params.size());
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
      sb.text_lines.clear();
      init();
    }
    
    virtual void calc_pre_draw(str::Adjustment adjustment) override
    {
      for (size_t p_idx = 0; p_idx < N; ++p_idx)
      {
        sb[p_idx] = params[p_idx]->str();
      }
    
      TextBox::calc_pre_draw(adjustment);
    }
  };

}

