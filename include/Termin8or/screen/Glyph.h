//
//  Glyph.h
//  Termin8or
//
//  Created by Rasmus Anthin on 2026-01-07.
//

#pragma once
#include "TermHelper.h"


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
      // 1) No fallback without preferred.
      assert(!(preferred == none32 && fallback != none)
             && "ERROR in Glyph(char32_t, char) : Fallback without preferred.");
      
      // 2) Fallback must be ASCII if present.
      assert(!(fallback != none && static_cast<unsigned char>(fallback) > 0x7F)
             && "ERROR in Glyph(char32_t, char) : Fallback must be ASCII (<=0x7F).");
      
      // 3) Preferred must be ASCII if fallback is not present.
      assert(!(preferred != none32 && preferred > 0x7F && fallback == none)
             && "ERROR in Glyph(char32_t, char) : Preferred cannot be non-ASCII while fallback is unset.");
             
      // Canonicalize ASCII glyphs.
      if (preferred != none32 && preferred <= 0x7F)
        fallback = static_cast<char>(preferred);
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
          pos += toks_len;
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
          pos += toks_len;
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
  };
  
}
