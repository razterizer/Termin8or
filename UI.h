//
//  UI.h
//  Termin8or
//
//  Created by Rasmus Anthin on 2024-07-08.
//

#pragma once
#include <Core/StringBox.h>

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
    bool framed_mode = true;
  };
  
  // ////

  class TextBox
  {
  protected:
    str::StringBox sb;
    size_t N = 0;
    size_t len_max = 0;
    std::vector<styles::Style> line_styles;
    
    void init()
    {
      N = sb.size();
    }
    
  public:
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
    
    void set_text(const std::vector<std::string>& text_lines, const std::vector<styles::Style>& styles = {})
    {
      sb = str::StringBox { text_lines };
      line_styles = styles;
      init();
      if (text_lines.size() != line_styles.size())
        line_styles.clear();
    }
    
    void set_text(const std::string& text)
    {
      sb = str::StringBox { text };
      init();
    }
    
    void set_text(const std::string& text, styles::Style style)
    {
      sb = str::StringBox { text };
      line_styles.clear();
      line_styles.emplace_back(style);
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
    void draw(SpriteHandler<NR, NC>& sh, const TextBoxDrawingArgsPos& args)
    {
      auto pos = args.pos;
      const styles::Style& box_style = args.base.box_style;
      bool draw_box_outline = args.base.draw_box_outline;
      bool draw_box_bkg = args.base.draw_box_bkg;
      int box_padding_ud = args.base.box_padding_ud;
      int box_padding_lr = args.base.box_padding_lr;
      std::optional<styles::Style> box_outline_style = args.base.box_outline_style;
      drawing::OutlineType outline_type = args.base.outline_type;
      for (size_t l_idx = 0; l_idx < N; ++l_idx)
        sh.write_buffer(sb[l_idx], pos.r + static_cast<int>(l_idx), pos.c, line_styles.empty() ? box_style : line_styles[l_idx]);
      if (draw_box_outline)
        drawing::draw_box_outline(sh, pos.r - 1 - box_padding_ud, pos.c - 1 - box_padding_lr, static_cast<int>(N + 1 + 2*box_padding_ud), static_cast<int>(len_max + 1 + 2*box_padding_lr), outline_type, box_outline_style.value_or(box_style));
      if (draw_box_bkg)
        drawing::draw_box(sh, pos.r - 1 - box_padding_ud, pos.c - 1 - box_padding_lr, static_cast<int>(N + 1 + 2*box_padding_ud), static_cast<int>(len_max + 1 + 2*box_padding_lr), box_style);
    }
    
    template<int NR, int NC>
    void draw(SpriteHandler<NR, NC>& sh, const TextBoxDrawingArgsAlign& args)
    {
      int box_padding_ud = args.base.box_padding_ud;
      int box_padding_lr = args.base.box_padding_lr;
    
      RC pos { 0, 0 };
      
      auto mid_v = std::round((NR - N)*0.5f) - ((NR - N)%2 == 1)*0.5f;
      auto mid_h = std::round((NC - len_max)*0.5f) - ((NC - len_max)%2 == 1)*0.5f;
      
      switch (args.v_align)
      {
        case VerticalAlignment::TOP:
          pos.r = 2 + box_padding_ud - !args.framed_mode;
          break;
        case VerticalAlignment::CENTER:
          pos.r = mid_v;
          break;
        case VerticalAlignment::BOTTOM:
          pos.r = 2*mid_v - box_padding_ud - 2 + !args.framed_mode;
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
          pos.c = 2*mid_h - box_padding_lr - 2 + !args.framed_mode;
          break;
      }
      
      TextBoxDrawingArgsPos pargs;
      pargs.pos = pos;
      pargs.base = args.base;
      
      draw(sh, pargs);
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

