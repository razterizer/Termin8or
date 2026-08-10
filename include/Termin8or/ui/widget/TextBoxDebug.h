//
//  TextBoxDebug.h
//  Termin8or
//
//  Created by Rasmus Anthin on 2026-05-10.
//

#pragma once
#include "TextBox.h"
#include "../../geom/RC.h"
#include <string>

#define PARAM(var) #var, &var

namespace t8x
{

  using RC = t8::RC;

  // /////////////////////////////////////////////////////////////
  //  _____                          ____                       //
  // |  __ \                        |  _ \                      //
  // | |__) |_ _ _ __ __ _ _ __ ___ | |_) | __ _ ___  ___       //
  // |  ___/ _` | '__/ _` | '_ ` _ \|  _ < / _` / __|/ _ \      //
  // | |  | (_| | | | (_| | | | | | | |_) | (_| \__ \  __/      //
  // |_|   \__,_|_|  \__,_|_| |_| |_|____/ \__,_|___/\___|      //
  // /////////////////////////////////////////////////////////////
  
  struct ParamBase
  {
    std::string name;
  
    ParamBase(const std::string& a_name) : name(a_name) {}
    virtual ~ParamBase() = default;
    virtual std::string str() const = 0;
  };
  
  // /////////////////////////////////////////////////////////////
  //  _____       __ _____                                      //
  // |  __ \     / _|  __ \                                     //
  // | |__) |___| |_| |__) |_ _ _ __ __ _ _ __ ___              //
  // |  _  // _ \  _|  ___/ _` | '__/ _` | '_ ` _ \             //
  // | | \ \  __/ | | |  | (_| | | | (_| | | | | | |            //
  // |_|  \_\___|_| |_|   \__,_|_|  \__,_|_| |_| |_|            //
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
  //  _____                                                     //
  // |  __ \                                                    //
  // | |__) |_ _ _ __ __ _ _ __ ___                             //
  // |  ___/ _` | '__/ _` | '_ ` _ \                            //
  // | |  | (_| | | | (_| | | | | | |                           //
  // |_|   \__,_|_|  \__,_|_| |_| |_|                           //
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
  
  // /////////////////////////////////////////////////////////////////////
  //  _______        _   ____            _____       _                  //
  // |__   __|      | | |  _ \          |  __ \     | |                 //
  //    | | _____  _| |_| |_) | _____  _| |  | | ___| |__  _   _  __ _  //
  //    | |/ _ \ \/ / __|  _ < / _ \ \/ / |  | |/ _ \ '_ \| | | |/ _` | //
  //    | |  __/>  <| |_| |_) | (_) >  <| |__| |  __/ |_) | |_| | (_| | //
  //    |_|\___/_/\_\\__|____/ \___/_/\_\_____/ \___|_.__/ \__,_|\__, | //
  //                                                              __/ | //
  //                                                             |___/  //
  // /////////////////////////////////////////////////////////////////////
  
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
