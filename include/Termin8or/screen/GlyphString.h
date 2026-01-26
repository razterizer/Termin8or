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
    GlyphString() = default;
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
  
  namespace operators
  {
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

}
