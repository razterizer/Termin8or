//
//  Gradient.h
//  Termin8or
//
//  Created by Rasmus Anthin on 2024-09-02.
//

#pragma once
#include <Core/Math.h>
#include <assert.h>


template<typename T>
struct Gradient
{
  std::vector<std::pair<float, T>> gradient;
  T default_value;
  
  Gradient(T def = T{}) : default_value(def) {}
  Gradient(const std::vector<std::pair<float, T>>& a_gradient, T def = T{})
    : gradient(a_gradient), default_value(def)
  {}
  
  T operator() (float t) const
  {
    assert(0.f <= t && t <= 1.f);
    float t_dist_min = 2.f;
    T closest_obj = default_value; //Color::Default;
    for (const auto& obj_pair : gradient)
    {
      auto t_dist = std::abs(t - obj_pair.first);
      if (math::minimize(t_dist_min, t_dist))
        closest_obj = obj_pair.second;
    }
    return closest_obj;
  }
};
