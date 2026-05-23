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
  
  // Canonicalization here means that if preferred is ASCII,
  //   then fallback will be set to be the same value as preferred.
  // In try_canonicalize_from_fallback() the order goes the other way, meaning
  //   that if preferred == none32 and fallback is ASCII,
  //   then preferred will instead be assigned the value of fallback.
  // Uncanonicalization is only for the display strings and means that
  //   if preferred and fallback are the same, the fallback is hidden and
  //   only preferred is displayed.
  
  struct Glyph
  {
    static const char32_t none32 = term::none32;
    static const char none = term::none;
    
    char32_t preferred = none32;
    char fallback = none;
    
  private:
    static bool fails_1_fallback_wo_preferred(const Glyph& g) noexcept
    {
      bool has_cp = g.preferred != none32;
      bool has_fb = g.fallback != none;
      return !has_cp && has_fb;
    }
    
    static bool fails_2_existing_fallback_isnt_ascii(const Glyph& g) noexcept
    {
      bool has_fb = g.fallback != none;
      return has_fb && !g.is_fallback_printable_ascii();
    }
    
    static bool fails_3_existing_preferred_isnt_ascii_wo_fallback(const Glyph& g) noexcept
    {
      bool has_cp = g.preferred != none32;
      bool has_fb = g.fallback != none;
      return has_cp && !g.is_preferred_printable_ascii() && !has_fb;
    }
  
  public:
    Glyph() = default;
    
    Glyph(char32_t pref, char fb = none)
      : preferred(pref)
      , fallback(fb)
    {
      if (preferred == static_cast<char32_t>(none))
        preferred = none32;
        
      // 1) No fallback without preferred.
      assert(!fails_1_fallback_wo_preferred(*this)
             && "ERROR in Glyph(char32_t, char) : Fallback without preferred.");
      
      // 2) Fallback must be ASCII if present.
      assert(!fails_2_existing_fallback_isnt_ascii(*this)
             && "ERROR in Glyph(char32_t, char) : Fallback must be printable ASCII ([0x20, 0x7E]).");
      
      // 3) Preferred must be ASCII if fallback is not present.
      assert(!fails_3_existing_preferred_isnt_ascii_wo_fallback(*this)
             && "ERROR in Glyph(char32_t, char) : Preferred cannot be non-ASCII while fallback is unset.");
             
      // Canonicalize ASCII glyphs.
      if (preferred != none32 && is_preferred_printable_ascii())
        fallback = static_cast<char>(preferred);
    }
    
    bool is_preferred_printable_ascii() const
    {
      return is_printable_ascii(preferred);
    }
    
    bool is_fallback_printable_ascii() const
    {
      return is_printable_ascii(static_cast<unsigned char>(fallback));
    }
    
    bool try_canonicalize_from_fallback()
    {
      if (fails_1_fallback_wo_preferred(*this))
      {
        unsigned char ufb = static_cast<unsigned char>(fallback);
        
        if (is_printable_ascii(ufb))
        {
          preferred = static_cast<char32_t>(ufb);
          return true;
        }
      }
      return false;
    }
    
    inline bool empty() const noexcept
    {
      return empty_preferred();
    }
    
    inline bool empty_preferred() const noexcept
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
    
    inline bool valid() const noexcept
    {
      return !fails_1_fallback_wo_preferred(*this)
          && !fails_2_existing_fallback_isnt_ascii(*this)
          && !fails_3_existing_preferred_isnt_ascii_wo_fallback(*this);
    }
    
    bool valid_after_canonicalization() const
    {
      auto glyph = *this;
      glyph.try_canonicalize_from_fallback();
      
      return glyph.valid();
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
    // p != none32 && p printable ASCII => "[<p>]"
    // p != none32 && p not printable ASCII  => "[<p_hex>,<f>]" (because of invariant assert 3)
    //
    // legacy_ascii_only == true:
    // p == none32 && f == none => "?"
    // p != none32 && p printable ASCII => "<p>"
    // p != none32 && p not printable ASCII  => "<f>" (because of invariant assert 3)
    //
    // Round trips legacy_ascii_only == false:
    // p == 'A' && f == none => "[A]"  => p == 'A', f == 'A'
    // Round trips legacy_ascii_only == true:
    // p == none32 && f == none => "?" => p == '?', f == '?'
    // p == '?' && f == none => "?" => p == '?', f == '?'
    //
    // Glyph {none32, 'a'} is draft state; canonicalize before serializing.
    //
    // Serialize valid/canonical glyph.
    std::string str(bool legacy_ascii_only = false) const
    {
      assert(valid());
      
      std::string ret;
      if (legacy_ascii_only)
      {
        if (preferred == none32)
          return "?";
          
        if (is_preferred_printable_ascii())
          return std::string(1, static_cast<char>(preferred));
          
        assert(fallback != none && "Glyph::str(): fallback cannot be unset when preferred isn't printable ASCII!");
        assert(is_fallback_printable_ascii() && "Glyph::str(): fallback must be printable ASCII!");
        return std::string(1, fallback);
      }
      
      ret.push_back('[');
      if (preferred != none32)
      {
        if (is_preferred_printable_ascii())
          ret.push_back(static_cast<char>(preferred));
        else
        {
          ret += str::int2hex(static_cast<uint32_t>(preferred));
          
        assert(fallback != none && "ERROR in Glyph::str() : Fallback character cannot be unset when preferred isn't printable ASCII!");
          assert(is_fallback_printable_ascii() && "ERROR in Glyph::str() : Fallback character must be printable ASCII!");
          ret.push_back(',');
          ret.push_back(fallback);
        }
      }
      ret.push_back(']');
      return ret;
    }
    
    // legacy_ascii_only == false
    // "[]" => p == none32, f == none
    // "[A]" => p == A, f == A (canonicalized p -> f like the constructor)
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
        auto ch = static_cast<unsigned char>(substr[0]);
        if (!is_printable_ascii(ch))
          return false;
        auto tok = std::string(1, static_cast<char>(ch));
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
      
      auto success = [this]()
      {
        assert(valid_after_canonicalization());
        return true;
      };
      
      if (substr.starts_with("["))
      {
        auto end_idx = substr.find("]");
        if (end_idx == 1)
        {
          pos += 2;
          preferred = none32;
          fallback = none;
          return success();
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
            return error("A non-ASCII unicode code point must be followed by a printable ASCII!");
          f_set_preferred(tok0);
          fallback = static_cast<char>(preferred); // Canonicalize ASCII.
          pos += static_cast<int>(toks_len);
          return success();
        }
        else if (tokens.size() == 2)
        {
          f_set_preferred(tokens[0]);
          if (!is_preferred_printable_ascii())
          {
            if (tokens[1].size() != 1)
              return error("Fallback must be a single ASCII char!");
            f_set_fallback(tokens[1]);
            if (fallback == none || !is_fallback_printable_ascii())
              return error("Fallback must be printable ASCII!");
          }
          else
          {
            // ASCII preferred; ignore provided fallback and normalize.
            fallback = static_cast<char>(preferred);
          }
          pos += static_cast<int>(toks_len);
          return success();
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
        else if (is_preferred_printable_ascii())
          str_preferred = std::string(1, static_cast<char>(preferred));
        else
          str_preferred = "0x" + str::int2hex(preferred);
        str_fallback = fallback == none ? "" : std::string(1, fallback);
        width_preferred = str::lenI(str_preferred);
      }
      else if constexpr (std::is_same_v<CharT, char32_t>)
      {
        bool can_render_preferred = t8::term::can_render_single_column_cp_cached(preferred);
        bool can_encode_unicode = can_render_preferred && (is_preferred_printable_ascii() || !t8::term::force_ascii_fallback);
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
