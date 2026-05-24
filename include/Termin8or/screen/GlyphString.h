//
//  GlyphString.h
//  Termin8or
//
//  Created by Rasmus Anthin on 2026-01-26.
//

#pragma once
#include "Glyph.h"
#include <string_view>
#include <string>
#include <vector>


namespace t8
{

  struct GlyphString
  {
    using value_type = Glyph;
    static constexpr size_t npos = static_cast<size_t>(-1);
  
    GlyphString() = default;
    GlyphString(std::string_view sv) = delete;
    GlyphString(const std::string& str) = delete;
    // These could be made explicit, but that reduces ergonomy a lot.
    // keeping them implicit is not so bad after all.
    GlyphString(const Glyph& glyph)
    {
      glyph_vector.emplace_back(glyph);
    }
    GlyphString(char c)
    {
      glyph_vector.emplace_back(c, c);
    }
    
    static GlyphString from_ascii(std::string_view sv_ascii)
    {
      GlyphString gs;
      gs.glyph_vector.reserve(sv_ascii.size());
      for (unsigned char c : sv_ascii)
      {
        assert(str::is_printable_ascii(c) && "ERROR in GlyphString::from_ascii() : Characters must be printable ASCII!");
        gs.glyph_vector.emplace_back(c, c);
      }
      return gs;
    }
    
    static GlyphString from_utf8(std::string_view sv_utf8, char fallback = '?')
    {
      assert(str::is_printable_ascii(fallback) && "ERROR in GlyphString::from_utf8() : Fallback must be printable ASCII!");
      
      GlyphString gs;
      // Slight over-reserving, but I think it ought to be acceptable.
      gs.glyph_vector.reserve(sv_utf8.size());
      
      std::string utf8 { sv_utf8 };
      size_t byte_idx = 0;
      char32_t ch32 = utf8::none;
      while (utf8::decode_next_utf8_char32(utf8, ch32, byte_idx))
      {
        if (str::is_printable_ascii(ch32))
          gs.glyph_vector.emplace_back(ch32, static_cast<char>(ch32));
        else
          gs.glyph_vector.emplace_back(ch32, fallback);
      }
      return gs;
    }
    
    
    inline bool empty() const noexcept
    {
      return glyph_vector.empty();
    }
    
    inline size_t size() const noexcept
    {
      return glyph_vector.size();
    }
    
    inline size_t length() const noexcept
    {
      return glyph_vector.size();
    }
    
    inline size_t find(const Glyph& needle) const
    {
      size_t idx = 0;
      for (const auto& hay : glyph_vector)
      {
        if (hay == needle)
          return idx;
        ++idx;
      }
      return npos;
    }
    
    inline size_t find(const GlyphString& needle) const
    {
      const size_t Nh = glyph_vector.size();
      const size_t Nn = needle.glyph_vector.size();
      // E.g.
      // n: 012 (Nn = 3)
      // h: ABC0123 (Nh = 7)
      // matchings: 5.
      // ABC0123
      // 012
      // ABC0123
      //  012
      // ABC0123
      //   012
      // ABC0123
      //    012
      // ABC0123
      //     012
      // Nm = Nh - Nn + 1 = 7 - 3 + 1 = 5
      if (Nn == 0)
        return 0;
      if (Nn > Nh)
        return npos;
      auto Nm = Nh - Nn + 1; // Now guaranteed to be positive.
      for (size_t i = 0; i < Nm; ++i)
      {
        bool all_matches = true;
        for (size_t j = 0; j < Nn; ++j)
        {
          if (glyph_vector[i + j] != needle.glyph_vector[j])
          {
            all_matches = false;
            break;
          }
        }
        if (all_matches)
          return i;
      }
      return npos;
    }
    
    inline GlyphString substr(size_t pos, size_t count = npos) const
    {
      // pos = 5, count = 3
      // [0123456789ABCDEF]
      //       .--
      // [0123]
      //   .--
      
      assert(pos != npos);
      assert(pos <= glyph_vector.size());
      
      const size_t n = glyph_vector.size();
      size_t len = count;
      
      if (len == npos || pos + len > n)
        len = n - pos;

      GlyphString ret;
      ret.glyph_vector.insert(ret.glyph_vector.end(),
                              glyph_vector.begin() + static_cast<ptrdiff_t>(pos),
                              glyph_vector.begin() + static_cast<ptrdiff_t>(pos + len));
      return ret;
    }
    
    template<typename CharT>
    std::string encode() const
    {
      std::string str;
      for (const auto& g : glyph_vector)
        str += g.encode_single_width_glyph<CharT>();
      return str;
    }
    
    Glyph& operator[](int idx) { return glyph_vector[idx]; }
    const Glyph& operator[](int idx) const { return glyph_vector[idx]; }
    Glyph& operator[](size_t idx) { return glyph_vector[idx]; }
    const Glyph& operator[](size_t idx) const { return glyph_vector[idx]; }
    
    inline GlyphString& operator+=(const GlyphString& gs)
    {
      glyph_vector.reserve(glyph_vector.size() + gs.glyph_vector.size());
      for (const auto& g : gs.glyph_vector)
        glyph_vector.emplace_back(g);
      return *this;
    }
    
    inline GlyphString& operator+=(std::string_view sv) = delete;
    
    inline GlyphString& operator+=(char c)
    {
      glyph_vector.reserve(glyph_vector.size() + 1);
      glyph_vector.emplace_back(c, c);
      return *this;
    }
    
    inline GlyphString& operator+=(const Glyph& g)
    {
      glyph_vector.reserve(glyph_vector.size() + 1);
      glyph_vector.emplace_back(g);
      return *this;
    }
    
    // ///////////////////////////////////////////
  
    std::vector<Glyph> glyph_vector;
  };
  
  namespace literals
  {
    // "_gs" produces an ASCII part (no CharT, no ScreenHandler needed).
    inline GlyphString operator"" _gs(const char* s, size_t n)
    {
      return GlyphString::from_ascii(std::string_view { s, n });
    }
  }
  
  inline GlyphString operator+(GlyphString lhs, const GlyphString& rhs)
  {
    lhs.glyph_vector.reserve(lhs.glyph_vector.size() + rhs.glyph_vector.size());
    for (const auto& g : rhs.glyph_vector)
      lhs.glyph_vector.emplace_back(g);
    return lhs;
  }
  
  // Append ASCII.
  inline GlyphString operator+(GlyphString lhs, std::string_view rhs) = delete;
  
  // Append char.
  inline GlyphString operator+(GlyphString lhs, char rhs)
  {
    lhs.glyph_vector.reserve(lhs.glyph_vector.size() + 1);
    lhs.glyph_vector.emplace_back(rhs, rhs);
    return lhs;
  }
  
  // Append Glyph.
  inline GlyphString operator+(GlyphString lhs, const Glyph& rhs)
  {
    lhs.glyph_vector.reserve(lhs.glyph_vector.size() + 1);
    lhs.glyph_vector.emplace_back(rhs);
    return lhs;
  }
  
}


namespace str
{
  inline t8::GlyphString adjust_str(const t8::GlyphString& str, Adjustment adj, int width, int start_idx = 0, t8::Glyph empty_char = U' ')
  {
    return adjust_str<t8::GlyphString, t8::Glyph>(str,
                                                  adj,
                                                  width,
                                                  start_idx,
                                                  empty_char);
  }
}
