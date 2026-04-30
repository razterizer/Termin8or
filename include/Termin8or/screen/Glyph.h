//
//  Glyph.h
//  Termin8or
//
//  Created by Rasmus Anthin on 2026-01-07.
//

#pragma once
#include "TermHelper.h"
#include "StyledString.h"


namespace t8
{
  
  struct Glyph
  {
    static const char32_t none32 = term::none32;
    static const char none = term::none;
    
    char32_t preferred = none32;
    char fallback = none;
    
    Glyph() = default;
    
    Glyph(char32_t pref, char fb = none)
      : preferred(pref)
      , fallback(fb)
    {
      if (preferred == static_cast<char32_t>(none))
        preferred = none32;
    
      bool has_cp = preferred != none32;
      bool has_fb = fallback != none;
    
      // 1) No fallback without preferred.
      assert(!(!has_cp && has_fb)
             && "ERROR in Glyph(char32_t, char) : Fallback without preferred.");
      
      // 2) Fallback must be ASCII if present.
      assert(!(has_fb && static_cast<unsigned char>(fallback) > 0x7F)
             && "ERROR in Glyph(char32_t, char) : Fallback must be ASCII (<=0x7F).");
      
      // 3) Preferred must be ASCII if fallback is not present.
      assert(!(has_cp && preferred > 0x7F && !has_fb)
             && "ERROR in Glyph(char32_t, char) : Preferred cannot be non-ASCII while fallback is unset.");
             
      // Canonicalize ASCII glyphs.
      if (has_cp && preferred <= 0x7F)
        fallback = static_cast<char>(preferred);
    }
    
    bool try_canonicalize_from_fallback()
    {
      bool has_cp = preferred != none32;
      bool has_fb = fallback != none;
      
      if (!has_cp && has_fb)
      {
        unsigned char ufb = static_cast<unsigned char>(fallback);
        
        if (ufb <= 0x7F)
        {
          preferred = static_cast<char32_t>(ufb);
          return true;
        }
      }
      return false;
    }
    
    inline bool empty() const noexcept
    {
      return preferred == none32;
    }
    
    inline bool empty_fallback() const noexcept
    {
      return fallback == none;
    }

    inline bool fully_empty() const noexcept
    {
      return empty() && empty_fallback();
    }
    
    bool valid_after_canonicalization() const
    {
      auto glyph = *this;
      glyph.try_canonicalize_from_fallback();
      
      const bool has_cp = glyph.preferred != none32;
      const bool has_fb = glyph.fallback != none;
      const auto fb_u = static_cast<unsigned char>(glyph.fallback);
      
      return !(!has_cp && has_fb)
          && !(has_fb && fb_u > 0x7F)
          && !(has_cp && glyph.preferred > 0x7F && !has_fb);
    }
    
    inline void clear()
    {
      preferred = none32;
      fallback = none;
    }
    
    bool operator==(const Glyph& g) const
    {
      return this->preferred == g.preferred
          && this->fallback == g.fallback;
    }
    
    // legacy_ascii_only == false:
    // p == none32 && f == none => "[]"
    // p != none32 && p <= 0x7F => "[<p>]"
    // p != none32 && p > 0x7F  => "[<p_hex>,<f>]" (because of invariant assert 3)
    //
    // legacy_ascii_only == true:
    // p == none32 && f == none => "?"
    // p != none32 && p <= 0x7F => "<p>"
    // p != none32 && p > 0x7F  => "<f>" (because of invariant assert 3)
    //
    // Round trips legacy_ascii_only == false:
    // p == 'A' && f == none => "[A]"  => p == 'A', f == 'A'
    // Round trips legacy_ascii_only == true:
    // p == none32 && f == none => "?" => p == '?', f == '?'
    // p == '?' && f == none => "?" => p == '?', f == '?'
    std::string str(bool legacy_ascii_only = false) const
    {
      std::string ret;
      auto fb_u = static_cast<unsigned char>(fallback);
      if (legacy_ascii_only)
      {
        if (preferred == none32)
          return "?";
          
        if (preferred <= 0x7F)
          return std::string(1, static_cast<char>(preferred));
          
        assert(fallback != none && "Glyph::str(): fallback cannot be unset when preferred is > 0x7F");
        assert(fb_u <= 0x7F && "Glyph::str(): fallback must be <= 0x7F");
        return std::string(1, fallback);
      }
      
      ret.push_back('[');
      if (preferred != none32)
      {
        if (preferred <= 0x7F)
          ret.push_back(static_cast<char>(preferred));
        else
        {
          ret += str::int2hex(static_cast<uint32_t>(preferred));
          
          assert(fallback != none && "ERROR in Glyph::str() : Fallback character cannot be unset when preferred is > 0x7F!");
          assert(fb_u <= 0x7F && "ERROR in Glyph::str() : Fallback character must be <= 0x7F!");
          ret.push_back(',');
          ret.push_back(fallback);
        }
      }
      ret.push_back(']');
      return ret;
    }
    
    // legacy_ascii_only == false
    // "[]" => p == none32, f == none
    // "[A]" => p == A, f == none
    // "[2603]" => error
    // "[A,A]" => p == A, f == A (not outputed by str() though)
    // "[A,2603]" => error
    // "[2603,A]" => p == 2603, f == A
    // "[2603,2603]" => error
    //
    // "?" => p == ?, f == ? (p == none32 && f == none outputs "?" though)
    // "A" => p == A, f == A
    bool parse(const std::string& str, int& pos, bool legacy_ascii_only = false, bool verbose = true)
    {
      // abcd[2603,e]fghi
      // 0123456789ABCDEF
      // pos = 4
      // substr = [2603,e]fghi
      //          0123456789AB
      // end_idx = 7
      // toks_str = substr.substr(0, end_idx + 1) = substr.substr(0, 8)
      // toks_str = [2603,e]
      //            01234567
      
      auto f_set_preferred = [&](const std::string& token)
      {
        auto tok_len = token.length();
        if (tok_len == 1)
          preferred = token[0];
        else
          preferred = str::hex2int(token);
      };
      
      auto f_set_fallback = [&](const std::string& token)
      {
        auto tok_len = token.length();
        if (tok_len == 1)
          fallback = token[0];
      };
      
      if (pos < 0 || static_cast<size_t>(pos) >= str.size())
        return false;
      auto substr = str.substr(pos);
      if (legacy_ascii_only)
      {
        auto tok = std::string(1, substr[0]);
        f_set_preferred(tok);
        f_set_fallback(tok);
        pos++;
        return true;
      }
      
      auto error = [&](const char* msg)
      {
        if (verbose)
          std::cerr << "ERROR in Glyph::parse(str) for str @ " << pos << " : " << msg << '\n';
        return false;
      };
      
      if (substr.starts_with("["))
      {
        auto end_idx = substr.find("]");
        if (end_idx == 1)
        {
          pos += 2;
          preferred = none32;
          fallback = none;
          return true;
        }
        if (end_idx == std::string::npos)
          return error("Unable to find matching end bracket \"]\"!");
        auto toks_len = end_idx + 1;
        auto toks_str = substr.substr(0, toks_len);
        auto tokens = str::tokenize(toks_str, { ',', '[', ']' });
        //for (const auto& tok : tokens)
        //  std::cout << tok << std::endl;
        if (tokens.size() == 1)
        {
          const auto& tok0 = tokens[0];
          if (tok0.length() > 1)
            return error("A unicode code point > 0x7F must be followed by an ASCII (<= 0x7F) character!");
          f_set_preferred(tok0);
          fallback = static_cast<char>(preferred); // Normalize ASCII.
          pos += static_cast<int>(toks_len);
          return true;
        }
        else if (tokens.size() == 2)
        {
          f_set_preferred(tokens[0]);
          if (preferred > 0x7F)
          {
            if (tokens[1].size() != 1)
              return error("Fallback must be a single ASCII char!");
            f_set_fallback(tokens[1]);
            auto fb_u = static_cast<unsigned char>(fallback);
            if (fallback == none || fb_u > 0x7F)
              return error("Fallback must be ASCII (<=0x7F)!");
          }
          else
          {
            // ASCII preferred; ignore provided fallback and normalize.
            fallback = static_cast<char>(preferred);
          }
          pos += static_cast<int>(toks_len);
          return true;
        }
        else
          return error("Wrong number of tokens within bracketed glyph scope!");
      }
      return error("Unable to find a bracketed glyph scope!");
    }
    
    bool parse(const std::string& str, bool legacy_ascii_only = false, bool verbose = true)
    {
      int pos = 0;
      return parse(str, pos, legacy_ascii_only, verbose);
    }
    
    template<typename CharT>
    inline std::string encode_single_width_glyph() const
    {
      return t8::term::encode_single_width_glyph<CharT>(preferred, fallback);
    }
    
    template<typename CharT>
    std::vector<t8::StyledString> format_long(bool has_preferred,
                                              bool uncanonicalize_fallback,
                                              const Style& preferred_style,
                                              const Style& fallback_style,
                                              const Style& bracket_style) const
    {
      std::string str_preferred;
      std::string str_fallback;
      int width_preferred = 0;
      int width_fallback = 0;
      if constexpr (std::is_same_v<CharT, char>)
      {
        if (preferred == none32)
          str_preferred = "";
        else if (preferred < 0x7F)
          str_preferred = std::string(1, static_cast<char>(preferred));
        else
          str_preferred = "0x" + str::int2hex(preferred);
        str_fallback = fallback == none ? "" : std::string(1, fallback);
        width_preferred = str::lenI(str_preferred);
      }
      else if constexpr (std::is_same_v<CharT, char32_t>)
      {
        bool can_render_preferred = t8::term::can_render_single_column_cp_cached(preferred);
        bool can_encode_unicode = can_render_preferred && (preferred < 0x7F || !t8::term::force_ascii_fallback);
        if (!has_preferred || preferred == none32)
          str_preferred = "";
        else if (can_encode_unicode)
          str_preferred = encode_single_width_glyph<CharT>();
        else
          str_preferred = "0x" + str::int2hex(preferred);
        str_fallback = fallback == none ? "" : std::string(1, fallback);
        width_preferred = str_preferred.empty() ? 0 : (can_encode_unicode ? 1 : str::lenI(str_preferred));
      }
      width_fallback = str::lenI(str_fallback);
      
      if (uncanonicalize_fallback && fallback != none && static_cast<char32_t>(fallback) == preferred)
      {
        return {
          { "[", bracket_style, 1 },
          { str_preferred, preferred_style, width_preferred },
          { "|", bracket_style, 1 },
          { "]", bracket_style, 1 }
        };
      }
      
      return {
        { "[", bracket_style, 1 },
        { str_preferred, preferred_style, width_preferred },
        { "|", bracket_style, 1 },
        { str_fallback, fallback_style, width_fallback },
        { "]", bracket_style, 1 }
      };
    }
    
    template<typename CharT>
    std::vector<t8::StyledString> format_short(bool uncanonicalize_fallback,
                                               const Style& preferred_style,
                                               const Style& fallback_style,
                                               const Style& bracket_style) const
    {
      auto sstr = format_long<CharT>(preferred != none32,
                                     uncanonicalize_fallback,
                                     preferred_style, fallback_style, bracket_style);
      stlutils::erase_at(sstr, 2);
      if (sstr[1].text.starts_with("0x"))
        sstr[1].text = "#";
      return sstr;
    }
    
  };
  
}
