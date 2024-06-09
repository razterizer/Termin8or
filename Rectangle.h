#pragma once

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
  
    bool is_inside(int rr, int cc)
    {
      return r <= rr && rr <= r + r_len && c <= cc && cc <= c + c_len;
    }
  };
  
}
