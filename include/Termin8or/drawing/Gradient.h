//
//  Gradient.h
//  Termin8or
//
//  Created by Rasmus Anthin on 2024-09-02.
//

#pragma once
#include <Core/MathUtils.h>
#include <assert.h>

namespace t8x
{
  
  // Param values define the centers of the ranges for the values.
  //      t0            t1           t2     t3       : param at center of value range.
  //  0    :             :            :      :    1
  //  |---------|------------------|-----|--------|
  //  |<------->|<---------------->|<--->|<------>|
  //      v0             v1           v2      v3     : value ranges.
  //
  // For equidistant params: t = (1/N:1/N:1)-1/(2*N)
  //   where N is the number of gradient nodes.
  
  template<typename T>
  struct Gradient
  {
    std::vector<std::pair<float, T>> gradient;
    T default_value;
    
    Gradient(T def = T{}) : default_value(def) {}
    Gradient(const std::vector<std::pair<float, T>>& a_gradient, T def = T{})
      : gradient(a_gradient), default_value(def)
    {}
    Gradient(const std::vector<T>& uniform_gradient, T def = T{})
    {
      auto dt = 1.f / uniform_gradient.size();
      auto t = dt - dt/2;
      for (const auto& obj : uniform_gradient)
      {
        gradient.emplace_back(t, obj);
        t += dt;
      }
    }
    
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

}
