#pragma once
#include <Core/Math.h>
#include <Core/Vec2.h>
#include <string>


namespace t8
{
  
  struct RC
  {
    RC() = default;
    RC(int rr, int cc) : r(rr), c(cc) {}
    
    int r = -1;
    int c = -1;
    
    bool operator==(const RC& pt) const { return r == pt.r && c == pt.c; }
    bool operator!=(const RC& pt) const { return !(*this == pt); }
    
    bool operator<(const RC& other) const
    {
      if (r != other.r)
        return r < other.r;
      return c < other.c;
    }
    
    const RC& operator+() const
    {
      return *this;
    }
    
    RC operator-() const
    {
      return { -r, -c };
    }
    
    RC operator+(const RC& pt) const
    {
      return { r + pt.r, c + pt.c };
    }
    RC operator-(const RC& pt) const
    {
      return { r - pt.r, c - pt.c };
    }
    RC& operator+=(const RC& pt)
    {
      this->r += pt.r;
      this->c += pt.c;
      return *this;
    }
    RC& operator-=(const RC& pt)
    {
      this->r -= pt.r;
      this->c -= pt.c;
      return *this;
    }
    RC operator*(int v) const
    {
      return { r * v, c * v };
    }
    RC operator/(int v) const
    {
      return { r / v, c / v };
    }
    RC& operator/=(int v)
    {
      this->r /= v;
      this->c /= v;
      return *this;
    }
    
    RC abs() const
    {
      return { static_cast<int>(std::abs(r)), static_cast<int>(std::abs(c)) };
    }
    
    RC clamp(int r_min, int r_max, int c_min, int c_max)
    {
      RC rc;
      rc.r = math::clamp<int>(r, r_min, r_max);
      rc.c = math::clamp<int>(c, c_min, c_max);
      return rc;
    }
    
    std::string str() const
    {
      return "(" + std::to_string(r) + ", " + std::to_string(c) + ")";
    }
  };
  
  static inline float distance(const RC& ptA, const RC& ptB)
  {
    return math::distance<float>(static_cast<float>(ptA.r),
                                 static_cast<float>(ptA.c),
                                 static_cast<float>(ptB.r),
                                 static_cast<float>(ptB.c));
  }
  
  static inline float distance_squared(const RC& ptA, const RC& ptB)
  {
    return math::distance_squared<float>(static_cast<float>(ptA.r),
                                         static_cast<float>(ptA.c),
                                         static_cast<float>(ptB.r),
                                         static_cast<float>(ptB.c));
  }
  
  Vec2 to_Vec2(const RC& p)
  {
    Vec2 v;
    v.r = static_cast<float>(p.r);
    v.c = static_cast<float>(p.c);
    return v;
  }
  
  RC to_RC_round(const Vec2& v) { return { math::roundI(v.r), math::roundI(v.c) }; }
  
  RC to_RC_floor(const Vec2& v) { return { static_cast<int>(v.r), static_cast<int>(v.c) }; }
  
}
