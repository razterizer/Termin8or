#pragma once
#include <Core/Math.h>

struct RC
{
  RC() = default;
  RC(int rr, int cc) : r(rr), c(cc) {}
  
  int r = -1;
  int c = -1;
  
  bool operator==(const RC& pt) const { return r == pt.r && c == pt.c; }
  bool operator!=(const RC& pt) const { return !(*this == pt); }
  
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
};

static float distance(const RC& ptA, const RC& ptB)
{
  return math::distance<float>(ptA.r, ptA.c, ptB.r, ptB.c);
}
