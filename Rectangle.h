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
    
    bool is_inside(const RC& pos) const
    {
      return is_inside(pos.r, pos.c);
    }
    
    bool is_inside_offs(int rr, int cc, int offs) const
    {
      return r - offs <= rr && rr <= r + r_len + offs && c - offs <= cc && cc <= c + c_len + offs;
    }
    
    bool is_inside_offs(const RC& pos, int offs) const
    {
      return is_inside_offs(pos.r, pos.c, offs);
    }
    
    bool is_inside_offs(int rr, int cc, int r_offs, int c_offs) const
    {
      return r - r_offs <= rr && rr <= r + r_len + r_offs && c - c_offs <= cc && cc <= c + c_len + c_offs;
    }
    
    bool is_inside_offs(const RC& pos, int r_offs, int c_offs) const
    {
      return is_inside_offs(pos.r, pos.c, r_offs, c_offs);
    }
    
    bool is_inside_offs(int rr, int cc, int top_offs, int bottom_offs, int left_offs, int right_offs) const
    {
      return r - top_offs <= rr && rr <= r + r_len + bottom_offs && c - left_offs <= cc && cc <= c + c_len + right_offs;
    }
    
    bool is_inside_offs(const RC& pos, int top_offs, int bottom_offs, int left_offs, int right_offs) const
    {
      return is_inside_offs(pos.r, pos.c, top_offs, bottom_offs, left_offs, right_offs);
    }
    
    bool is_on_border(int rr, int cc) const
    {
      return r == rr || rr == r + r_len || c == cc || cc == c + c_len;
    }
    
    bool is_on_border(const RC& pos) const
    {
      return is_on_border(pos.r, pos.c);
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
    
    RC pos() const { return { r, c }; }
    RC size() const { return { r_len, c_len }; }
    
    void set_pos(const RC& rc_pos)
    {
      r = rc_pos.r;
      c = rc_pos.c;
    }
    
    void set_size(const RC& rc_size)
    {
      r_len = rc_size.r;
      c_len = rc_size.c;
    }
  };
  
}
