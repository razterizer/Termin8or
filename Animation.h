//
//  Animation.h
//  Termin8or
//
//  Created by Rasmus Anthin on 2024-11-18.
//

#pragma once
#include <Core/Math.h>
#include <functional>


namespace easings
{
  std::function<float(float)> ease_in_sine = [](float t)
  {
    return 1.f - std::cos((t * math::c_pi) * 0.5f);
  };
  
  std::function<float(float)> ease_out_sine = [](float t)
  {
    return std::sin((t * math::c_pi) * 0.5f);
  };
}


struct TransitionAnimation
{
  float ease_in_start_time_s = 0.f;
  float ease_in_end_time_s = 0.f;
  float ease_out_start_time_s = 0.f;
  float ease_out_end_time_s = 0.f;
  
  float animate(float time_s, float value_start, float value_stationary, float value_end,
                std::function<float(float)>& ease_in_func, std::function<float(float)>& ease_out_func) const
  {
    float t_ease_in = math::value_to_param(time_s, ease_in_start_time_s, ease_in_end_time_s);
    if (math::in_range<float>(t_ease_in, {}, 0.f, Range::FreeOpen))
      return value_start;
    if (math::in_range<float>(t_ease_in, 0.f, 1.f, Range::ClosedOpen))
      return math::lerp(ease_in_func(t_ease_in), value_start, value_stationary);
    float t_ease_out = math::value_to_param(time_s, ease_out_start_time_s, ease_out_end_time_s);
    if (math::in_range<float>(t_ease_out, 0.f, 1.f, Range::ClosedOpen))
      return math::lerp(ease_out_func(t_ease_out), value_stationary, value_end);
    if (math::in_range<float>(t_ease_out, 1.f, {}, Range::ClosedFree))
      return value_end;
    return value_stationary;
  }
  
  bool done(float time_s) const
  {
    float t_ease_out = math::value_to_param(time_s, ease_out_start_time_s, ease_out_end_time_s);
    return math::in_range<float>(t_ease_out, 1.f, {}, Range::ClosedFree);
  }
};
