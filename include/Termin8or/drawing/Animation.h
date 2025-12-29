//
//  Animation.h
//  Termin8or
//
//  Created by Rasmus Anthin on 2024-11-18.
//

#pragma once
#include <Core/MathUtils.h>
#include <functional>


namespace t8x
{

  namespace easings
  {
    std::function<float(float)> ease_lin = [](float t)
    {
      return t;
    };
    
    std::function<float(float)> ease_in_sine = [](float t)
    {
      return 1.f - std::cos((t * math::c_pi) * 0.5f);
    };
    
    std::function<float(float)> ease_out_sine = [](float t)
    {
      return std::sin((t * math::c_pi) * 0.5f);
    };
  }
  
  
  struct TransitionAnimationInOut
  {
    float transition_start_time_s = 0.f;
    // Relative to transition_start_time_s.
    float enter_rel_start_time_s = 0.f;
    float enter_rel_end_time_s = 0.f;
    float exit_rel_start_time_s = 0.f;
    float exit_rel_end_time_s = 0.f;
    
    float animate(float time_s, float value_start, float value_stationary, float value_end,
                  std::function<float(float)>& ease_enter_func = easings::ease_lin,
                  std::function<float(float)>& ease_exit_func = easings::ease_lin) const
    {
      auto rel_time_s = time_s - transition_start_time_s;
      
      float t_enter = math::value_to_param(rel_time_s, enter_rel_start_time_s, enter_rel_end_time_s);
      if (math::in_range(t_enter, {}, 0.f, Range::FreeOpen))
        return value_start;
      if (math::in_range(t_enter, 0.f, 1.f, Range::ClosedOpen))
        return math::lerp(ease_enter_func(t_enter), value_start, value_stationary);
      float t_exit = math::value_to_param(rel_time_s, exit_rel_start_time_s, exit_rel_end_time_s);
      if (math::in_range(t_exit, 0.f, 1.f, Range::ClosedOpen))
        return math::lerp(ease_exit_func(t_exit), value_stationary, value_end);
      if (math::in_range(t_exit, 1.f, {}, Range::ClosedFree))
        return value_end;
      return value_stationary;
    }
    
    bool in_range(float time_s) const
    {
      auto rel_time_s = time_s - transition_start_time_s;
      float t = math::value_to_param(rel_time_s, enter_rel_start_time_s, exit_rel_end_time_s);
      return math::in_range(t, 0.f, 1.f, Range::ClosedOpen);
    }
    
    bool begun(float time_s) const
    {
      return time_s - transition_start_time_s >= enter_rel_start_time_s;
    }
    
    bool done(float time_s) const
    {
      return time_s - transition_start_time_s >= exit_rel_end_time_s;
    }
  };
  
  
  struct TransitionAnimationSingle
  {
    float transition_start_time_s = 0.f;
    // Relative to transition_start_time_s.
    float enter_rel_start_time_s = 0.f;
    float exit_rel_end_time_s = 0.f;
    
    float animate(float time_s, float value_start, float value_end,
                  std::function<float(float)>& ease_func = easings::ease_lin) const
    {
      auto rel_time_s = time_s - transition_start_time_s;
      
      float t = math::value_to_param(rel_time_s, enter_rel_start_time_s, exit_rel_end_time_s);
      if (math::in_range(t, {}, 0.f, Range::FreeOpen))
        return value_start;
      if (math::in_range(t, 1.f, {}, Range::ClosedFree))
        return value_end;
      return math::lerp(ease_func(t), value_start, value_end);
    }
    
    bool in_range(float time_s) const
    {
      auto rel_time_s = time_s - transition_start_time_s;
      float t = math::value_to_param(rel_time_s, enter_rel_start_time_s, exit_rel_end_time_s);
      return math::in_range(t, 0.f, 1.f, Range::ClosedOpen);
    }
    
    bool begun(float time_s) const
    {
      return time_s - transition_start_time_s >= enter_rel_start_time_s;
    }
    
    bool done(float time_s) const
    {
      return time_s - transition_start_time_s >= exit_rel_end_time_s;
    }
  };
  
}
