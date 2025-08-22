//
//  UI.h
//  Termin8or
//
//  Created by Rasmus Anthin on 2024-07-08.
//

#pragma once
#include "RC.h"
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
    int v_align_offs = 0;
    int h_align_offs = 0;
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
    std::vector<std::pair<RC, styles::Style>> override_textel_styles;
    
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
      if (draw_box_outline)
        drawing::draw_box_outline(sh, pos.r - 1 - box_padding_ud, pos.c - 1 - box_padding_lr, static_cast<int>(N + 2 + 2*box_padding_ud), static_cast<int>(len_max + 2 + 2*box_padding_lr), outline_type, box_outline_style.value_or(box_style));
      if (draw_box_bkg)
        drawing::draw_box(sh, pos.r - 1 - box_padding_ud, pos.c - 1 - box_padding_lr, static_cast<int>(N + 2 + 2*box_padding_ud), static_cast<int>(len_max + 2 + 2*box_padding_lr), box_style);
    }
    
    template<int NR, int NC>
    void draw(ScreenHandler<NR, NC>& sh, const TextBoxDrawingArgsAlign& args)
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
      
      pos.r += args.v_align_offs;
      pos.c += args.h_align_offs;
      
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

