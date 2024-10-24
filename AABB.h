//
//  AABB.h
//  Termin8or
//
//  Created by Rasmus Anthin on 2024-10-24.
//

#pragma once
#include "Rectangle.h"

template<typename T>
class AABB
{
  T rmin = math::get_max<T>();
  T rmax = math::get_min<T>();
  T cmin = math::get_max<T>();
  T cmax = math::get_min<T>();
  
  constexpr T offs() const { return static_cast<T>(std::is_integral<T>() ? 1 : 0); }

public:
  AABB() = default;
  
  AABB(T r, T c, T r_len, T c_len)
  {
    rmin = r;
    cmin = c;
    rmax = r + r_len - offs();
    cmax = c + c_len - offs();
  }
  
  AABB(const ttl::Rectangle& rectangle)
  {
    rmin = static_cast<T>(rectangle.r);
    cmin = static_cast<T>(rectangle.c);
    rmax = rmin + static_cast<T>(rectangle.r_len - offs());
    cmax = cmin + static_cast<T>(rectangle.c_len - offs());
  }
  
  AABB(const AABB& aabb)
  {
    rmin = aabb.rmin;
    cmin = aabb.cmin;
    rmax = aabb.rmax;
    cmax = aabb.cmax;
  }
  
  std::pair<T, T> p0() const { return { rmin, cmin }; }
  
  std::pair<T, T> p1() const { return { rmax, cmax }; }
  
  void set_empty()
  {
    rmin = math::get_max<T>();
    rmax = math::get_min<T>();
    cmin = math::get_max<T>();
    cmax = math::get_min<T>();
  }
  
  bool empty() const
  {
    return rmax < rmin || cmax < cmin;
  }
  
  void add_point(T r, T c)
  {
    math::minimize(rmin, r);
    math::maximize(rmax, r);
    math::minimize(cmin, c);
    math::maximize(cmax, c);
  }
  
  void add_point(const RC& pt)
  {
    add_point(static_cast<T>(pt.r), static_cast<T>(pt.c));
  }
  
  bool is_inside(T r, T c) const
  {
    return math::in_range(r, rmin, rmax, Range::Closed)
      && math::in_range(c, cmin, cmax, Range::Closed);
  }
  
  bool is_inside(const RC& pt) const
  {
    return is_inside(static_cast<T>(pt.r), static_cast<T>(pt.c));
  }
  
  bool overlaps(const AABB<T>& aabb) const
  {
    return !(aabb.rmax < this->rmin
      || this->rmax < aabb.rmin
      || aabb.cmax < this->cmin
      || this->cmax < aabb.cmin);
  }
  
  ttl::Rectangle to_rectangle() const
  {
    return
    {
      static_cast<int>(rmin), static_cast<int>(cmin),
      static_cast<int>(rmax - rmin + offs()), static_cast<int>(cmax - cmin + offs())
    };
  }
  
  template<typename Ret>
  AABB<Ret> convert() const
  {
    AABB<Ret> aabb
    {
      static_cast<Ret>(math::roundI(static_cast<float>(rmin))),
      static_cast<Ret>(math::roundI(static_cast<float>(cmin))),
      static_cast<Ret>(math::roundI(static_cast<float>(rmax - rmin + offs()))),
      static_cast<Ret>(math::roundI(static_cast<float>(cmax - cmin + offs()))),
    };
    return aabb;
  }


};
