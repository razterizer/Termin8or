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
  
    GlyphString() = default;
    // Consider these for clarity.
    // GlyphString::from_bytes(std::string_view) (current behavior)
    // GlyphString::from_utf8(std::string_view) (decode to Glyph(preferred_unicode, ascii_fallback))
    GlyphString(std::string_view sv)
    {
      glyph_vector.reserve(sv.size());
      for (unsigned char c : sv)
        glyph_vector.emplace_back(c, c);
    }
    GlyphString(const std::string& str)
    {
      glyph_vector.reserve(str.size());
      for (unsigned char c : str)
        glyph_vector.emplace_back(c, c);
    }
    GlyphString(const Glyph& glyph)
    {
      glyph_vector.emplace_back(glyph);
    }
    GlyphString(char c)
    {
      glyph_vector.emplace_back(c, c);
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
    
    // Returns npos if not found. npos here is a hack. Doesn't really belong here.
    inline size_t find(const Glyph& needle) const
    {
      size_t idx = 0;
      for (const auto& hay : glyph_vector)
      {
        if (hay == needle)
          return idx;
        ++idx;
      }
      return std::string::npos;
    }
    
    // Returns npos if not found. npos here is a hack. Doesn't really belong here.
    inline size_t find(const GlyphString& needle) const
    {
      auto Nh = glyph_vector.size();
      auto Nn = glyph_vector.size();
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
      // Nm = Nh - Nn - 1
      if (Nn >= Nh)
        return std::string::npos;
      auto Nd = Nh - Nn - 1; // Now guaranteed to be positive or zero.
      for (size_t i = 0; i < Nd; ++i)
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
      return std::string::npos;
    }
    
    // npos here is a hack. Doesn't really belong here.
    inline GlyphString substr(size_t pos, size_t count = std::string::npos) const
    {
      // pos = 5, count = 3
      // [0123456789ABCDEF]
      //       .--
      // [0123]
      //   .--
      
      assert(pos != std::string::npos);
      assert(pos <= glyph_vector.size());
      
      const size_t n = glyph_vector.size();
      size_t len = count;
      
      if (len == std::string::npos || pos + len > n)
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
    
    inline GlyphString& operator+=(const GlyphString& gs)
    {
      glyph_vector.reserve(glyph_vector.size() + gs.glyph_vector.size());
      for (const auto& g : gs.glyph_vector)
        glyph_vector.emplace_back(g);
      return *this;
    }
    
    inline GlyphString& operator+=(std::string_view sv)
    {
      glyph_vector.reserve(glyph_vector.size() + sv.size());
      for (unsigned char c : sv)
        glyph_vector.emplace_back(c, c);
      return *this;
    }
    
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
      GlyphString gs;
      gs.glyph_vector.reserve(n);
      for (size_t i = 0; i < n; ++i)
      {
        auto b = static_cast<unsigned char>(s[i]);
        gs.glyph_vector.emplace_back(b, b);
      }
      return gs;
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
  inline GlyphString operator+(GlyphString lhs, std::string_view rhs)
  {
    lhs.glyph_vector.reserve(lhs.glyph_vector.size() + rhs.size());
    for (unsigned char c : rhs)
      lhs.glyph_vector.emplace_back(c, c);
    return lhs;
  }
  
  // Append char.
  inline GlyphString operator+(GlyphString lhs, char rhs)
  {
    lhs.glyph_vector.reserve(lhs.glyph_vector.size() + 1);
    lhs.glyph_vector.emplace_back(rhs, rhs);
    return lhs;
  }
  
  // Append char32_t.
  inline GlyphString operator+(GlyphString lhs, char32_t rhs)
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
  t8::GlyphString adjust_str(const t8::GlyphString& str, Adjustment adj, int width, int start_idx = 0, t8::Glyph empty_char = U' ')
  {
    return adjust_str<t8::GlyphString, t8::Glyph>(str,
                                                  adj,
                                                  width,
                                                  start_idx,
                                                  empty_char);
  }
}
