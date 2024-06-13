#pragma once
#include "RC.h"

namespace ttl
{
  
  struct Rectangle
  {
    Rectangle() = default;
    Rectangle(int rr, int cc, int rr_len, int cc_len)
      : r(rr), c(cc), r_len(rr_len), c_len(cc_len)
    {}
    
    int r = 0;
    int c = 0;
    int r_len = 0; // #NOTE: Downwards height!
    int c_len = 0;
  
    bool is_inside(int rr, int cc) const
    {
      return r <= rr && rr <= r + r_len && c <= cc && cc <= c + c_len;
    }
    
    bool is_empty() const
    {
      return r_len == 0 && c_len == 0;
    }
    
    RC center() const
    {
      RC pc;
      pc.r = r + r_len/2;
      pc.c = c + c_len/2;
      return pc;
    }
    
    int top() const { return r; }
    int bottom() const { return r + r_len; }
    int left() const { return c; }
    int right() const { return c + c_len; }
  };
  
}
