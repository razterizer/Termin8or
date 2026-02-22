//
//  TermHelper.h
//  Termin8or
//
//  Created by Rasmus Anthin on 2026-01-19.
//

#pragma once
#include <Core/Utf8.h>
#include <Core/System.h>
#include <Core/Term.h>
#include <Core/MathUtils.h>


namespace t8
{
  
  namespace term
  {
    inline constexpr char32_t none32 = 0xFFFFFFFFu; // char32_t is unsigned.
    inline constexpr char none = -0x80;
    
    inline bool force_ascii_fallback = false;
    
    inline ::term::TermMode m_term_mode;
  
    inline void init_locale()
    {
      static bool once [[maybe_unused]] = []()
      {
        std::setlocale(LC_CTYPE, "");
        return true;
      }();
    }
    
    inline bool use_ansi_renderer()
    {
      return ::term::use_ansi_renderer(m_term_mode);
    }
    
    inline void emit_text(std::string_view sv_utf8, std::string_view sv_bytes_for_legacy = {})
    {
      ::term::emit_text(m_term_mode, sv_utf8, sv_bytes_for_legacy);
    }
    
    // We assume single column and rely on encoder fallback.
    inline bool is_single_column(char32_t cp)
    {
      if (!term::use_ansi_renderer())
      {
        // Treat as single-column if we can encode it to CP437 (or ASCII).
        // If you don’t have a predicate, just allow and rely on encoding fallback.
        return cp <= 0x10FFFF;
      }
      
      if (cp > 0x10FFFF)
        return false;
      
#ifdef _WIN32
      // If not cmd, assume single-column and rely on fallback policy elsewhere.
      return cp <= 0x10FFFF;
#else
      init_locale();
      wchar_t wc = static_cast<wchar_t>(cp);
      int w = ::wcwidth(wc);
      return w == 1; //w <= 1;
#endif
    }
    
    inline bool can_render_single_column_cp(char32_t cp)
    {
      // Reject invalid.
      if (cp > 0x10FFFF)
        return false;
      if (0xD800 <= cp && cp <= 0xDFFF)
        return false;
      
      if (m_term_mode.is_conhost_like)
      {
        // WinAPI buffer is BMP-only.
        if (cp > 0xFFFF)
          return false;
          
        // Conservative whitelist: box + block + ASCII.
        const bool ascii = math::in_r_c<char32_t>(cp, 0x20, 0x7E);
        const bool box   = math::in_r_c<char32_t>(cp, 0x2500, 0x257F);
        const bool block = math::in_r_c<char32_t>(cp, 0x2580, 0x259F);
          
        if (ascii || box || block)
          return true;
        
        switch (m_term_mode.win_font_class)
        {
          case ::term::WinFontClass::Unknown:
            return ascii;
          case ::term::WinFontClass::RasterTerminal:
            assert(!m_term_mode.truetype_font);
            return ascii;
          case ::term::WinFontClass::LucidaConsole:
            // #FIXME: Implement me.
            return ascii || box || block;
          case ::term::WinFontClass::ConsolasLike:
            // #FIXME: Implement me.
            return ascii || box || block;
          case ::term::WinFontClass::UnknownTrueType:
            //return ascii; // Perhaps too restrictive.
            return ascii || box || block; // More practical.
        }
        return true;
      }
      return is_single_column(cp);
    }
    
    inline char32_t get_renderable_char32(char32_t cp)
    {
      return can_render_single_column_cp(cp) ? cp : none32;
    }
    
    inline char32_t resolve_single_width_glyph(char32_t preferred, char fallback = none)
    {
      if (can_render_single_column_cp(preferred))
        return preferred;
          
      // Fallback (treat fallback as ASCII only).
      if (fallback != none)
      {
        unsigned char fb = static_cast<unsigned char>(fallback);
        if (fb <= 0x7F && can_render_single_column_cp(static_cast<char32_t>(fb)))
          return static_cast<char32_t>(fb);
      }
      
      return none32;
    }
    
    template<typename CharT>
    inline CharT resolve_single_width_glyph(char32_t preferred,
                                            char fallback = none)
    {
      auto f_handle_ascii = [preferred, fallback]() -> char
      {
        if (preferred <= 0x7F)
          return static_cast<char>(preferred);
        if (fallback != none && static_cast<unsigned char>(fallback) <= 0x7F)
          return fallback;
        return '?';
      };
    
      if constexpr (std::is_same_v<CharT, char>)
        return f_handle_ascii();
      if constexpr (std::is_same_v<CharT, char32_t>)
      {
        if (force_ascii_fallback)
          return static_cast<char32_t>(static_cast<unsigned char>(f_handle_ascii()));
        return term::resolve_single_width_glyph(preferred, fallback);
      }
      
      static_assert(std::is_same_v<CharT, char> || std::is_same_v<CharT, char32_t>,
                    "ERROR in ScreenHandler::resolve_single_width_glyph() : Unsupported CharT!");
      return static_cast<CharT>('?');
    }
    
    inline std::string encode_single_width_glyph(char32_t preferred,
                                                 char fallback = none)
    {
      char32_t cp = resolve_single_width_glyph(preferred, fallback);
      
      // Use 437 for cmd.exe for now as we only have a mapping from UTF-8 to CP437 atm.
      return utf8::encode_char32_utf8(cp);
    }
    
    template<typename CharT>
    inline std::string encode_single_width_glyph(char32_t preferred,
                                                 char fallback = none)
    {
      auto f_handle_ascii = [preferred, fallback]() -> std::string
      {
        if (preferred <= 0x7F)
          return std::string(1, static_cast<char>(preferred));
        if (fallback != none && static_cast<unsigned char>(fallback) <= 0x7F)
          return std::string(1, fallback);
        return "?";
      };
    
      if constexpr (std::is_same_v<CharT, char>)
        return f_handle_ascii();
      if constexpr (std::is_same_v<CharT, char32_t>)
      {
        if (force_ascii_fallback)
          return f_handle_ascii();
        return term::encode_single_width_glyph(preferred, fallback);
      }
      
      static_assert(std::is_same_v<CharT, char> || std::is_same_v<CharT, char32_t>,
                    "ERROR in ScreenHandler::encode_single_width_glyph() : Unsupported CharT!");
      return "?";
    }
    
  }
  
}
