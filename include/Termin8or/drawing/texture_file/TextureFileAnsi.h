//
//  TextureFileAnsi.h
//  Termin8or
//
//  Created by Rasmus Anthin on 2026-05-14.
//


#pragma once
#include "TextureFileCommon.h"
#include "../Texture.h"
#include "../../screen/Ansi.h"
#include <Core/TextIO.h>
#include <Core/StringHelper.h>
#include <Core/FolderHelper.h>
#include <fstream>
#include <sstream>


namespace t8
{

  enum class AnsiLoadGlyphEncoding
  {
    Auto,
    UTF8,
    CP437,
  };

  enum class AnsiSaveGlyphEncoding
  {
    AutoPreserveGlyphs,  // CP437 if lossless, else UTF8 + BOM. If *.utf8ans, always UTF8.
    AutoPreferCP437,     // CP437, using fallback for non-CP437 glyphs. If *.utf8ans, always UTF8.
    UTF8,                // Explicit UTF8. No BOM for *.utf8ans, otherwise BOM.
    CP437,               // Explicit CP437. Errors if preferred CPs cannot be converted to CP437.
  };
  
  inline bool is_ext_ansi_cp437(const std::string& ext)
  {
    return ext == "ans" || ext == "ansi" || ext == "txt" || ext == "diz" || ext == "asc" || ext == "nfo";
  }

  class TextureFileAnsi
  {
    // BOM (Byte Order Mark).
    // UTF-8 : EF BB BF
    static bool has_utf8_bom(const std::vector<std::string>& lines)
    {
      if (!lines.empty() && lines[0].size() >= 3)
      {
        const unsigned char b0 = static_cast<unsigned char>(lines[0][0]);
        const unsigned char b1 = static_cast<unsigned char>(lines[0][1]);
        const unsigned char b2 = static_cast<unsigned char>(lines[0][2]);
        
        if (b0 == 0xEF && b1 == 0xBB && b2 == 0xBF)
          return true;
      }
      return false;
    }
    
    static void strip_utf8_bom(std::vector<std::string>& lines)
    {
      lines[0].erase(0, 3);
    }
    
    static void add_utf8_bom(std::vector<std::string>& lines)
    {
      const char b0 = static_cast<char>(0xEF);
      const char b1 = static_cast<char>(0xBB);
      const char b2 = static_cast<char>(0xBF);
      std::string bstr;
      bstr.push_back(b0);
      bstr.push_back(b1);
      bstr.push_back(b2);
      
      if (lines.empty())
        lines.emplace_back().insert(0, bstr);
      else
        lines[0].insert(0, bstr);
    }

    static bool is_sauce_record_at(const std::string& bytes, size_t pos)
    {
      // SAUCE part is 128 bytes long:
      //   [art data][optional 0x1A][SAUCE00 ... 121 bytes sauce data].
      if (pos + 128 > bytes.size())
        return false;
      const std::string sauce_str = "SAUCE00";
      if (bytes.compare(pos, sauce_str.length(), sauce_str) != 0) // 7 + 121 = 128.
        return false;

      // 0..6    ID = "SAUCE00"
      // 7..41   Title
      // 42..61  Author
      // 62..81  Group
      // 82..89  Date
      // 90..93  FileSize
      // 94      DataType [0, 8]
      // 95      FileType
      // 96..99  TInfo1/TInfo2
      // ...
      //
      // Sanity check that DataType is in the range [0, 8] (usually 1 = Character).
      // 0 = None
      // 1 = Character
      // 2 = Bitmap
      // 3 = Vector
      // 4 = Audio
      // 5 = BinaryText
      // 6 = XBin
      // 7 = Archive
      // 8 = Executable
      const unsigned char data_type = static_cast<unsigned char>(bytes[pos + 94]);
      return data_type <= 8;
    }

    static uint16_t read_sauce_le_uint16(const std::string& bytes, size_t pos)
    {
      const auto lo = static_cast<unsigned char>(bytes[pos]);
      const auto hi = static_cast<unsigned char>(bytes[pos + 1]);
      return static_cast<uint16_t>(lo | (hi << 8));
    }

    static bool read_sauce_ansi_size(const std::string& bytes, size_t pos, RC& size)
    {
      if (!is_sauce_record_at(bytes, pos))
        return false;

      // SAUCE character files store width/height in TInfo1/TInfo2.
      const unsigned char data_type = static_cast<unsigned char>(bytes[pos + 94]);
      if (data_type != 1)
        return false;

      const int cols = read_sauce_le_uint16(bytes, pos + 96);
      const int rows = read_sauce_le_uint16(bytes, pos + 98);
      if (cols <= 0 && rows <= 0)
        return false;

      size = { rows, cols };
      return true;
    }

    static bool find_sauce_ansi_size(const std::string& bytes, RC& size)
    {
      const std::string sauce_str = "SAUCE00";
      bool found = false;
      for (size_t pos = bytes.find(sauce_str);
           pos != std::string::npos;
           pos = bytes.find(sauce_str, pos + sauce_str.length()))
      {
        if (read_sauce_ansi_size(bytes, pos, size))
          found = true;
      }
      return found;
    }

    static size_t find_inline_sauce_record_end(const std::string& bytes, size_t pos)
    {
      // Sauce record ends either at end of file or after 128 bytes.
      //   If '\0' not found then return npos.
      //   If '\0' found after the 128 bytes, then return npos as well.
      const size_t search_end = std::min(bytes.size(), pos + 128);
      if (bytes.find('\0', pos) >= search_end)
        return std::string::npos;

      // On Windows CRLF CRLF marks the end of the file.
      const std::string CRLFCRLF = "\r\n\r\n";
      auto end = bytes.find(CRLFCRLF, pos);
      if (end != std::string::npos && end < search_end)
        return end + CRLFCRLF.length();

      // Standard POSIX file ending with extra LF.
      const std::string LFLF = "\n\n";
      end = bytes.find(LFLF, pos);
      if (end != std::string::npos && end < search_end)
        return end + LFLF.length();

      return std::string::npos;
    }

    static void strip_sauce_records(std::string& bytes)
    {
      const std::string sauce_str = "SAUCE00";
      for (size_t pos = bytes.find(sauce_str);
           pos != std::string::npos;
           pos = bytes.find(sauce_str, pos))
      {
        if (is_sauce_record_at(bytes, pos))
          bytes.erase(pos, 128);
        else if (auto end = find_inline_sauce_record_end(bytes, pos);
                 end != std::string::npos)
          bytes.erase(pos, end - pos);
        else
          pos += sauce_str.length();
      }
    }
    
    static char ansi_fallback_for_unicode(char32_t cp)
    {
      switch (cp)
      {
          // Blocks and shades.
        case 0x2588: return '#'; // █ Full block
        case 0x2589: return '#'; // ▉ Left seven eighths block
        case 0x258A: return '#'; // ▊ Left three quarters block
        case 0x258B: return '#'; // ▋ Left five eighths block
        case 0x258C: return '|'; // ▌ Left half block
        case 0x258D: return '|'; // ▍ Left three eighths block
        case 0x258E: return '|'; // ▎ Left one quarter block
        case 0x258F: return '|'; // ▏ Left one eighth block
          
        case 0x2590: return '|'; // ▐ Right half block
        case 0x2591: return '.'; // ░ Light shade
        case 0x2592: return ':'; // ▒ Medium shade
        case 0x2593: return '#'; // ▓ Dark shade
          
        case 0x2580: return '-'; // ▀ Upper half block
        case 0x2584: return '_'; // ▄ Lower half block
          
          // Light box drawing.
        case 0x2500: return '-'; // ─
        case 0x2502: return '|'; // │
        case 0x250C: return '+'; // ┌
        case 0x2510: return '+'; // ┐
        case 0x2514: return '+'; // └
        case 0x2518: return '+'; // ┘
        case 0x251C: return '+'; // ├
        case 0x2524: return '+'; // ┤
        case 0x252C: return '+'; // ┬
        case 0x2534: return '+'; // ┴
        case 0x253C: return '+'; // ┼
          
          // Heavy box drawing.
        case 0x2501: return '-'; // ━
        case 0x2503: return '|'; // ┃
        case 0x250F: return '+'; // ┏
        case 0x2513: return '+'; // ┓
        case 0x2517: return '+'; // ┗
        case 0x251B: return '+'; // ┛
        case 0x2523: return '+'; // ┣
        case 0x252B: return '+'; // ┫
        case 0x2533: return '+'; // ┳
        case 0x253B: return '+'; // ┻
        case 0x254B: return '+'; // ╋
          
          // Double box drawing.
        case 0x2550: return '='; // ═
        case 0x2551: return '|'; // ║
        case 0x2552: return '+'; // ╒
        case 0x2553: return '+'; // ╓
        case 0x2554: return '+'; // ╔
        case 0x2555: return '+'; // ╕
        case 0x2556: return '+'; // ╖
        case 0x2557: return '+'; // ╗
        case 0x2558: return '+'; // ╘
        case 0x2559: return '+'; // ╙
        case 0x255A: return '+'; // ╚
        case 0x255B: return '+'; // ╛
        case 0x255C: return '+'; // ╜
        case 0x255D: return '+'; // ╝
        case 0x255E: return '+'; // ╞
        case 0x255F: return '+'; // ╟
        case 0x2560: return '+'; // ╠
        case 0x2561: return '+'; // ╡
        case 0x2562: return '+'; // ╢
        case 0x2563: return '+'; // ╣
        case 0x2564: return '+'; // ╤
        case 0x2565: return '+'; // ╥
        case 0x2566: return '+'; // ╦
        case 0x2567: return '+'; // ╧
        case 0x2568: return '+'; // ╨
        case 0x2569: return '+'; // ╩
        case 0x256A: return '+'; // ╪
        case 0x256B: return '+'; // ╫
        case 0x256C: return '+'; // ╬
          
          // Common symbols.
        case 0x00A0: return ' '; // Non-breaking space
        case 0x2022: return '*'; // • Bullet
        case 0x00B7: return '.'; // · Middle dot
        case 0x2219: return '.'; // ∙ Bullet operator
        case 0x25A0: return '#'; // ■ Black square
        case 0x25AA: return '#'; // ▪ Black small square
        case 0x25AB: return '#'; // ▫ White small square
        case 0x00AC: return '-'; // ¬ Not sign
        case 0x00B0: return 'o'; // ° Degree sign
        case 0x00B1: return '+'; // ± Plus-minus sign
        case 0x00B2: return '2'; // ² Superscript two
        case 0x00BC: return '1'; // ¼ Vulgar fraction one quarter
        case 0x00BD: return '1'; // ½ Vulgar fraction one half
        case 0x00D7: return 'x'; // × Multiplication sign
        case 0x00F7: return '/'; // ÷ Division sign
        case 0x0192: return 'f'; // ƒ Latin small letter f with hook
        case 0x207F: return 'n'; // ⁿ Superscript latin small letter n
        case 0x221A: return 'v'; // √ Square root
        case 0x221E: return '8'; // ∞ Infinity
        case 0x2229: return 'n'; // ∩ Intersection
        case 0x2248: return '~'; // ≈ Almost equal to
        case 0x2261: return '='; // ≡ Identical to
        case 0x2264: return '<'; // ≤ Less-than or equal to
        case 0x2265: return '>'; // ≥ Greater-than or equal to
        case 0x2310: return '-'; // ⌐ Reversed not sign
        case 0x2320: return '('; // ⌠ Top half integral
        case 0x2321: return ')'; // ⌡ Bottom half integral
          
          // CP437 accented letters, approximated for ASCII fallback mode.
        case 0x00AA: return 'a'; // ª Feminine ordinal indicator
        case 0x00BA: return 'o'; // º Masculine ordinal indicator
        case 0x00A1: return '!'; // ¡ Inverted exclamation mark
        case 0x00A3: return 'L'; // £ Pound sign
        case 0x00AB: return '<'; // « Left-pointing double angle quotation mark
        case 0x00BB: return '>'; // » Right-pointing double angle quotation mark
        case 0x00BF: return '?'; // ¿ Inverted question mark
        case 0x00B5: return 'u'; // µ Micro sign
        case 0x00C4: return 'A'; // Ä
        case 0x00C5: return 'A'; // Å
        case 0x00C6: return 'A'; // Æ
        case 0x00C7: return 'C'; // Ç
        case 0x00C9: return 'E'; // É
        case 0x00D1: return 'N'; // Ñ
        case 0x00D6: return 'O'; // Ö
        case 0x00D8: return 'O'; // Ø
        case 0x00DC: return 'U'; // Ü
        case 0x00DF: return 'B'; // ß
        case 0x00E0: return 'a'; // à
        case 0x00E1: return 'a'; // á
        case 0x00E2: return 'a'; // â
        case 0x00E4: return 'a'; // ä
        case 0x00E5: return 'a'; // å
        case 0x00E6: return 'a'; // æ
        case 0x00E7: return 'c'; // ç
        case 0x00E8: return 'e'; // è
        case 0x00E9: return 'e'; // é
        case 0x00EA: return 'e'; // ê
        case 0x00EB: return 'e'; // ë
        case 0x00EC: return 'i'; // ì
        case 0x00ED: return 'i'; // í
        case 0x00EE: return 'i'; // î
        case 0x00EF: return 'i'; // ï
        case 0x00F1: return 'n'; // ñ
        case 0x00F2: return 'o'; // ò
        case 0x00F3: return 'o'; // ó
        case 0x00F4: return 'o'; // ô
        case 0x00F6: return 'o'; // ö
        case 0x00F8: return 'o'; // ø
        case 0x00F9: return 'u'; // ù
        case 0x00FA: return 'u'; // ú
        case 0x00FB: return 'u'; // û
        case 0x00FC: return 'u'; // ü
        case 0x00FF: return 'y'; // ÿ
          
          // CP437 Greek/math letters, approximated for ASCII fallback mode.
        case 0x0393: return 'G'; // Γ
        case 0x0398: return 'O'; // Θ
        case 0x03A3: return 'S'; // Σ
        case 0x03A6: return 'O'; // Φ
        case 0x03A9: return 'O'; // Ω
        case 0x03B1: return 'a'; // α
        case 0x03B4: return 'd'; // δ
        case 0x03B5: return 'e'; // ε
        case 0x03C0: return 'p'; // π
        case 0x03C3: return 's'; // σ
        case 0x03C4: return 't'; // τ
        case 0x03C6: return 'o'; // φ
        
          // Special glyphs.
        case 0xFFFD: return '?'; // �
        
          // Low CP437 bytes.
        case 0x263A: return '@'; // ☺ smiling face
        case 0x263B: return '@'; // ☻ inverse smiling face
        case 0x2665: return 'v'; // ♥ heart
        case 0x2666: return '*'; // ♦ diamond
        case 0x2663: return '&'; // ♣ club
        case 0x2660: return '^'; // ♠ spade
        //case 0x2022: return '*'; // • bullet
        case 0x25D8: return 'o'; // ◘ inverse bullet
        case 0x25CB: return 'o'; // ○ white circle
        case 0x25D9: return 'o'; // ◙ inverse white circle
        case 0x2642: return 'M'; // ♂ male sign
        case 0x2640: return 'F'; // ♀ female sign
        case 0x266A: return '~'; // ♪ eighth note
        case 0x266B: return '~'; // ♫ beamed eighth notes
        case 0x263C: return '*'; // ☼ white sun
        
        case 0x25BA: return '>'; // ► black right-pointing pointer
        case 0x25C4: return '<'; // ◄ black left-pointing pointer
        case 0x2195: return '|'; // ↕ up-down arrow
        case 0x203C: return '!'; // ‼ double exclamation mark
        case 0x00B6: return 'P'; // ¶ pilcrow sign
        case 0x00A7: return 'S'; // § section sign
        case 0x25AC: return '-'; // ▬ black rectangle
        case 0x21A8: return '|'; // ↨ up-down arrow with base
        case 0x2191: return '^'; // ↑ up arrow
        case 0x2193: return 'v'; // ↓ down arrow
        case 0x2192: return '>'; // → right arrow
        case 0x2190: return '<'; // ← left arrow
        case 0x221F: return 'L'; // ∟ right angle
        case 0x2194: return '-'; // ↔ left-right arrow
        case 0x25B2: return '^'; // ▲ black up-pointing triangle
        case 0x25BC: return 'v'; // ▼ black down-pointing triangle
        
        case 0x2302: return 'H'; // ⌂ house

      }
      return Glyph::none;
    }
    
    static bool create_glyph_from_ansi(char32_t ch32, char fb, Glyph& g, bool verbose = true)
    {
      if (str::is_printable_ascii(ch32))
        g = Glyph { ch32 };
      else
      {
        if (fb != Glyph::none && !str::is_printable_ascii(static_cast<unsigned char>(fb)))
        {
          if (verbose)
            std::cerr << "ERROR in TextureFileAnsi::create_glyph_from_ansi() : Encountered a malformed fallback char.\n";
          return false;
        }
        char fallback = fb != Glyph::none ? fb : ansi_fallback_for_unicode(ch32);
        if (fallback == Glyph::none)
        {
          if (verbose)
          {
            std::cerr << "ERROR in TextureFileAnsi::create_glyph_from_ansi() : Missing fallback for Unicode glyph: U+"
                      << std::hex << std::uppercase << static_cast<uint32_t>(ch32)
                      << std::dec << std::nouppercase << ".\n";
          }
          return false;
        }
        
        g = Glyph { ch32, fallback };
      }
      return true;
    }
    
    static bool read_ansi_file(const std::string& file_path,
                               std::vector<std::string>& lines,
                               RC* sauce_size = nullptr,
                               int verbosity = 3)
    {
      std::ifstream file(file_path, std::ios::binary);
      if (!file.is_open())
      {
        if (verbosity >= 1)
          std::cerr << "ERROR in TextureFileAnsi::read_ansi_file() : Unable to open file \"" << file_path << "\"!" << std::endl;
        return false;
      }

      std::string bytes((std::istreambuf_iterator<char>(file)),
                         std::istreambuf_iterator<char>());
      if (bytes.empty())
      {
        if (verbosity >= 2)
          std::cerr << "ERROR in TextureFileAnsi::read_ansi_file() : End of file reached." << std::endl;
        return false;
      }

      if (sauce_size != nullptr)
        find_sauce_ansi_size(bytes, *sauce_size);

      strip_sauce_records(bytes);

      lines.clear();
      size_t line_start = 0;
      for (size_t i = 0; i < bytes.size(); ++i)
      {
        if (bytes[i] == '\n')
        {
          lines.emplace_back(bytes.substr(line_start, i - line_start));
          line_start = i + 1; // The +1 filters out '\n'.
        }
      }
      // If '\n' not found then add the remaining bytes.
      if (line_start < bytes.size())
        lines.emplace_back(bytes.substr(line_start));

      return true;
    }
    
    static bool has_non_cp437_convertible_glyphs_preferred(const Texture& tex)
    {
      for (auto& g : tex.glyphs)
      {
        if (g.empty())
          continue;
        if (!utf8::unicode_to_cp437(g.preferred).has_value())
          return true;
      }
      return false;
    }
    
  public:
    
    static bool load_ansi(Texture& tex, const std::string& file_path,
                          bool verbose = true,
                          AnsiLoadGlyphEncoding glyph_encoding = AnsiLoadGlyphEncoding::Auto,
                          Color ansi_default_fg = Color16::Default,
                          Color ansi_default_bg = Color16::Transparent2)
    {
      std::vector<std::string> lines;
      RC sauce_size;
      bool ret = read_ansi_file(file_path, lines, &sauce_size);
      if (!ret)
        return false;
      
      // Cases:
      // :w (warning), :c (correct).
      // *.ans, *.ansi (no BOM): Auto->CP437:c, UTF8:w, CP437:c
      // *.ans, *.ansi (has BOM): Auto->UTF8:c, UTF8:c, CP437:w
      // *.utf8ans (no BOM): Auto->UTF8:c, UTF8:c, CP437:w
      // *.utf8ans (has BOM): Auto->UTF8:c, UTF8:c, CP437:w (via BOM/CP437 conflict, not extension).
      auto ext = get_file_ext(file_path);
      bool utf8_bom = has_utf8_bom(lines);
      if (utf8_bom)
      {
        strip_utf8_bom(lines);
        
        if (glyph_encoding == AnsiLoadGlyphEncoding::Auto)
          glyph_encoding = AnsiLoadGlyphEncoding::UTF8;
        else if (glyph_encoding == AnsiLoadGlyphEncoding::CP437 && verbose)
          std::cerr << "WARNING in TextureFileAnsi::load_ansi() : Attempting to load a UTF-8 encoded ANSI file with CP437 encoding!\n";
      }
      
      if (glyph_encoding == AnsiLoadGlyphEncoding::Auto)
      {
        if (ext == "utf8ans")
          glyph_encoding = AnsiLoadGlyphEncoding::UTF8;
        else // *.ans, *.ansi.
          glyph_encoding = AnsiLoadGlyphEncoding::CP437;
      }
      else if (glyph_encoding == AnsiLoadGlyphEncoding::UTF8 && !utf8_bom)
      {
        if (verbose && is_ext_ansi_cp437(ext))
          std::cerr << "WARNING in TextureFileAnsi::load_ansi() : Attempting to load an ANSI file in UTF-8 encoding without UTF-8 BOM!\n";
      }
      else if (glyph_encoding == AnsiLoadGlyphEncoding::CP437 && !utf8_bom)
      {
        if (verbose && ext == "utf8ans")
          std::cerr << "WARNING in TextureFileAnsi::load_ansi() : Attempting to load a UTF-8 ANSI (*.utf8ans) file in CP437 encoding!\n";
      }
      
      // Normalizes DOS/Windows CRLF lines to plain content lines.
      for (auto& line : lines)
        if (!line.empty() && line.back() == '\r')
          line.pop_back();
      
      const int Nl = stlutils::sizeI(lines);
      for (int i = 0; i < Nl; ++i)
      {
        auto eof_pos = lines[i].find('\x1A');
        if (eof_pos != std::string::npos)
        {
          lines[i].erase(eof_pos);
          lines.resize(lines[i].empty() ? i : i + 1);
          break;
        }
      }
      
      const std::string empty_str_row;
      
      std::vector<std::string> fb_lines;
      auto fb_filepath = folder::join_filename_ext({ file_path, "fb" });
      TextIO::read_file(fb_filepath, fb_lines, verbose ? 2 : 1);
      
      std::vector<std::string> mat_lines;
      auto mat_filepath = folder::join_filename_ext({ file_path, "mat" });
      TextIO::read_file(mat_filepath, mat_lines, verbose ? 2 : 1);
        
      struct Cell
      {
        Glyph glyph;
        Color fg = Color16::Default;
        Color bg = Color16::Transparent2;
        uint8_t mat_raw = texture::raw_mat_none;
      };
      
      std::vector<std::vector<Cell>> rows;
      
      Color fg = ansi_default_fg;
      Color bg = ansi_default_bg;
      constexpr int ansi_terminal_width = 80;
      const int ansi_wrap_width = sauce_size.c > 0 ? sauce_size.c : ansi_terminal_width;
      const auto num_long_lines = static_cast<int>(stlutils::count_if(lines,
        [ansi_wrap_width](const auto& line) { return ansi_wrap_width < str::lenI(line); }));

      // Some old ANSI files split a logical 80-column stream over several
      //   physical file lines by saving the cursor before CRLF and restoring it
      //   at the start of the next line. That only renders correctly with wrap.
      const auto f_has_save_restore_continuation = [&]()
      {
        auto f_has_cursor_save_restore_at = [](const std::string& line, int pos, char command)
        {
          char parsed_command = '\0';
          return ansi::parse_ansi_cursor_save_restore(line, pos, parsed_command)
            && parsed_command == command;
        };
        
        for (int i = 0; i + 1 < stlutils::sizeI(lines); ++i)
        {
          const auto& line = lines[i];
          const auto& next_line = lines[i + 1];
          
          // Only treat save as continuation syntax when it is the final code
          //   on the physical line; saves in the middle are normal cursor state.
          const int save_pos = line.size() >= 3 ? stlutils::sizeI(line) - 3 : 0;
          const bool line_ends_with_save = f_has_cursor_save_restore_at(line, save_pos, 's')
            && save_pos + 3 == stlutils::sizeI(line);
          const bool next_line_starts_with_restore = f_has_cursor_save_restore_at(next_line, 0, 'u');

          if (line_ends_with_save && next_line_starts_with_restore)
            return true;
        }
        return false;
      }();
      const bool ansi_auto_wrap = sauce_size.c > 0
        || f_has_save_restore_continuation
        || (num_long_lines == 1 && stlutils::sizeI(lines) <= 2);
      
      auto f_make_blank_cell = [&]()
      {
        Cell cell;
        cell.glyph = Glyph { U' ' };
        cell.fg = ansi_default_fg;
        cell.bg = ansi_default_bg;
        return cell;
      };
      
      auto f_ensure_cursor_cell = [&](int r, int c)
      {
        while (stlutils::sizeI(rows) <= r)
          rows.emplace_back();
        
        auto& row = rows[r];
        while (stlutils::sizeI(row) <= c)
          row.emplace_back(f_make_blank_cell());
      };
      
      int cursor_r = 0;
      int cursor_c = 0;
      int saved_cursor_r = 0;
      int saved_cursor_c = 0;
      bool has_saved_cursor = false;
      bool bright_fg = false;
      auto f_wrap_cursor = [&]()
      {
        if (!ansi_auto_wrap)
          return;
        
        if (cursor_c < ansi_wrap_width)
          return;
        
        cursor_r += cursor_c / ansi_wrap_width;
        cursor_c = cursor_c % ansi_wrap_width;
      };
      
      int input_r = 0;
      const int num_input_rows = stlutils::sizeI(lines);
      for (const auto& line : lines)
      {
        const auto& fb_row = input_r < stlutils::sizeI(fb_lines) ? fb_lines[input_r] : empty_str_row;
        const auto& mat_row = input_r < stlutils::sizeI(mat_lines) ? mat_lines[input_r] : empty_str_row;
        
        size_t byte_idx = 0;
        char32_t ch32 = utf8::none;
        int fb_pos = 0;
        int mat_pos = 0;
        
        int len_line = str::lenI(line);
        for (int i = 0; i < len_line; )
        {
          byte_idx = static_cast<size_t>(i);
          
          if (line[i] == '\033')
          {
            std::vector<int> params;
            int next = i;
            if (ansi::parse_ansi_sgr_params(line, next, params))
            {
              ansi::apply_ansi_sgr_params(params, fg, bg,
                                          bright_fg,
                                          ansi_default_fg,
                                          ansi_default_bg);
              i = next;
              continue;
            }
            
            char cursor_dir = '\0';
            int cursor_count = 0;
            next = i;
            if (ansi::parse_ansi_cursor_move(line, next, cursor_dir, cursor_count))
            {
              switch (cursor_dir)
              {
                case 'A':
                  cursor_r = std::max(0, cursor_r - cursor_count);
                  break;
                case 'B':
                  cursor_r += cursor_count;
                  break;
                case 'C':
                  cursor_c += cursor_count;
                  f_wrap_cursor();
                  break;
                case 'D':
                  cursor_c = std::max(0, cursor_c - cursor_count);
                  break;
              }
              
              i = next;
              continue;
            }
            
            char erase_target = '\0';
            int erase_mode = 0;
            next = i;
            if (ansi::parse_ansi_erase(line, next, erase_target, erase_mode))
            {
              if (erase_target == 'J' && erase_mode == 2)
              {
                rows.clear();
                cursor_r = 0;
                cursor_c = 0;
                has_saved_cursor = false;
              }
              else if (erase_target == 'K' && erase_mode == 2)
              {
                while (stlutils::sizeI(rows) <= cursor_r)
                  rows.emplace_back();
                rows[cursor_r].resize(cursor_c);
              }
              
              i = next;
              continue;
            }
            
            char cursor_save_restore = '\0';
            next = i;
            if (ansi::parse_ansi_cursor_save_restore(line, next, cursor_save_restore))
            {
              if (cursor_save_restore == 's')
              {
                saved_cursor_r = cursor_r;
                saved_cursor_c = cursor_c;
                has_saved_cursor = true;
              }
              else if (has_saved_cursor)
              {
                cursor_r = saved_cursor_r;
                cursor_c = saved_cursor_c;
              }
              
              i = next;
              continue;
            }
            
            int cursor_row = 1;
            int cursor_col = 1;
            next = i;
            if (ansi::parse_ansi_cursor_position(line, next, cursor_row, cursor_col))
            {
              cursor_r = cursor_row - 1;
              cursor_c = cursor_col - 1;
              i = next;
              continue;
            }
            
            if (verbose)
            {
              std::cerr << "ERROR in TextureFileAnsi::load_ansi() : Unsupported ANSI escape sequence";
              if (i + 1 < len_line && line[i + 1] == '[')
              {
                std::cerr << ": \\033[";
                for (int j = i + 2; j < len_line; ++j)
                {
                  char ch = line[j];
                  if (str::is_printable_ascii(ch))
                    std::cerr << ch;
                  else
                    std::cerr << "0x" << str::int2hex(ch);
                  
                  if ('@' <= ch && ch <= '~')
                    break;
                }
                std::cerr << ".";
              }
              std::cerr << "\n";
            }
            return false;
          }
          
          bool decoded = false;
          
          unsigned char b = static_cast<unsigned char>(line[i]);
          if (glyph_encoding == AnsiLoadGlyphEncoding::UTF8)
          {
            if (b < 0x20)
            {
              ch32 = U' ';
              ++i;
              decoded = true;
            }
            else
            {
              decoded = utf8::decode_next_utf8_char32(line, ch32, byte_idx);
              i = static_cast<int>(byte_idx);
            }
          }
          else if (glyph_encoding == AnsiLoadGlyphEncoding::CP437)
          {
            auto cp = utf8::cp437_to_unicode(b);
            if (cp.has_value())
            {
              ch32 = cp.value();
              ++i;
              decoded = true;
            }
          }
          
          if (!decoded)
            return false;
          
          Cell cell;
          const auto fb = fb_pos < str::lenI(fb_row) ? fb_row[fb_pos] : Glyph::none;
          ++fb_pos;
          if (!create_glyph_from_ansi(ch32, fb, cell.glyph, verbose))
          {
            if (verbose)
              std::cerr << "ERROR in TextureFileAnsi::load_ansi() : Unable to create glyph object from ANSI file unicode bytes.\n";
            return false;
          }
          cell.mat_raw = texture::str_to_raw_mat(mat_row, mat_pos);
          cell.fg = fg;
          cell.bg = bg;
          
          f_ensure_cursor_cell(cursor_r, cursor_c);
          rows[cursor_r][cursor_c] = cell;
          ++cursor_c;
          f_wrap_cursor();
        }
        
        ++input_r;
        if (input_r < num_input_rows)
        {
          ++cursor_r;
          cursor_c = 0;
        }
      }
      
      int num_rows = stlutils::sizeI(rows);
      int num_cols = 0;
      for (const auto& row : rows)
        math::maximize(num_cols, stlutils::sizeI(row));

      tex.size = { num_rows, num_cols };
      tex.area = tex.size.r * tex.size.c;
      tex.glyphs.assign(tex.area, ' ');
      tex.fg_colors.assign(tex.area, Color16::Default);
      tex.bg_colors.assign(tex.area, Color16::Transparent2);
      tex.materials_raw.assign(tex.area, texture::raw_mat_none);
      
      for (int r = 0; r < num_rows; ++r)
      {
        int nc = stlutils::sizeI(rows[r]);
        for (int c = 0; c < nc; ++c)
        {
          int idx = tex.index(r, c);
          tex.glyphs[idx] = rows[r][c].glyph;
          tex.fg_colors[idx] = rows[r][c].fg;
          tex.bg_colors[idx] = rows[r][c].bg;
          tex.materials_raw[idx] = rows[r][c].mat_raw;
        }
      }
      
      return true;
    }
    
    static bool save_ansi(const Texture& tex, const std::string& file_path,
                          bool verbose = true,
                          AnsiSaveGlyphEncoding ansi_glyph_encoding = AnsiSaveGlyphEncoding::AutoPreserveGlyphs,
                          Color ansi_default_fg = Color16::Default,
                          Color ansi_default_bg = Color16::Transparent2)
    {
      std::vector<std::string> lines;
      
      auto ext = get_file_ext(file_path);
      bool utf8_bom = false;
      // Cases:
      // :w (warning), :c (correct).
      // *.ans, *.ansi: Auto->CP437, UTF8->BOM:c, CP437:c
      // *.utf8ans: Auto->UTF8, UTF8:c, CP437:w
      switch (ansi_glyph_encoding)
      {
        case AnsiSaveGlyphEncoding::AutoPreserveGlyphs:
          if (ext == "utf8ans")
            ansi_glyph_encoding = AnsiSaveGlyphEncoding::UTF8;
          else if (is_ext_ansi_cp437(ext))
          {
            if (has_non_cp437_convertible_glyphs_preferred(tex))
            {
              utf8_bom = true;
              ansi_glyph_encoding = AnsiSaveGlyphEncoding::UTF8;
            }
            else
              ansi_glyph_encoding = AnsiSaveGlyphEncoding::CP437;
          }
          else
          {
            std::cerr << "ERROR in TextureFileAnsi::save_ansi() : Unknown file extension. Unable to save Texture." << std::endl;
            return false;
          }
          break;
        case AnsiSaveGlyphEncoding::AutoPreferCP437:
          if (ext == "utf8ans")
            ansi_glyph_encoding = AnsiSaveGlyphEncoding::UTF8;
          break;
        case AnsiSaveGlyphEncoding::UTF8:
          if (ext != "utf8ans")
          {
            utf8_bom = true;
            if (!is_ext_ansi_cp437(ext) && verbose)
              std::cerr << "WARNING in TextureFileAnsi::save_ansi() : Attempting to save a UTF-8 ANSI file to a file with an unsupported extension!\n";
          }
          break;
        case AnsiSaveGlyphEncoding::CP437:
          if (ext == "utf8ans" && verbose)
            std::cerr << "WARNING in TextureFileAnsi::save_ansi() : Attempting to save a UTF-8 ANSI (*.utf8ans) file in CP437 encoding!\n";
          break;
      }
      
      std::vector<std::string> fb_lines;
      auto fb_filepath = folder::join_filename_ext({ file_path, "fb" });
      
      std::vector<std::string> mat_lines;
      auto mat_filepath = folder::join_filename_ext({ file_path, "mat" });
      
      Color curr_fg = ansi_default_fg;
      Color curr_bg = ansi_default_bg;
      
      bool encoding_ok = true;
      
      for (int r = 0; r < tex.size.r; ++r)
      {
        auto& line = lines.emplace_back();
        auto& fb_line = fb_lines.emplace_back();
        auto& mat_line = mat_lines.emplace_back();
        
        if (utf8_bom)
        {
          add_utf8_bom(lines);
          utf8_bom = false;
        }
        
        for (int c = 0; c < tex.size.c; ++c)
        {
          auto idx = tex.index(r, c);
          auto fg = tex.fg_colors[idx];
          auto bg = tex.bg_colors[idx];
          
          if (fg != curr_fg || bg != curr_bg)
          {
            line += ansi::colors_to_ansi_sgr_string(fg, bg, ansi_default_fg, ansi_default_bg);
            curr_fg = fg;
            curr_bg = bg;
          }
          
          const auto& g = tex.glyphs[idx];
          if (ansi_glyph_encoding == AnsiSaveGlyphEncoding::UTF8)
          {
            const bool valid_unicode = !g.empty()
                                    && g.preferred <= 0x10FFFF
                                    && !(0xD800 <= g.preferred && g.preferred <= 0xDFFF);
            
            if (valid_unicode)
              line += utf8::encode_char32_utf8(g.preferred);
            else if (!g.empty_fallback() && g.is_fallback_printable_ascii())
              line.push_back(g.fallback);
            else
            {
              if (verbose)
                std::cerr << "ERROR in TextureFileAnsi::save_ansi() : Unable to encode glyph " << g.str(true) << " in UTF-8 encoding!\n";
              encoding_ok = false;
            }
          }
          else if (ansi_glyph_encoding == AnsiSaveGlyphEncoding::CP437 ||
                   ansi_glyph_encoding == AnsiSaveGlyphEncoding::AutoPreferCP437)
          {
            auto ch_fb = g.fallback;
            auto cp = utf8::unicode_to_cp437(g.preferred);
            if (cp.has_value())
              line.push_back(static_cast<char>(cp.value()));
            else if (!g.empty_fallback() && str::is_printable_ascii(ch_fb) &&
                     ansi_glyph_encoding == AnsiSaveGlyphEncoding::AutoPreferCP437)
              line.push_back(ch_fb);
            else
            {
              if (verbose)
                std::cerr << "ERROR in TextureFileAnsi::save_ansi() : Unable to encode glyph " << g.str(true) << " in CP437 encoding!\n";
              encoding_ok = false;
            }
          }
          
          fb_line.push_back(g.fallback);
          mat_line += texture::raw_mat_to_str(tex.materials_raw[idx]);
        }
        
        line += "\033[0m";
        
        curr_fg = ansi_default_fg;
        curr_bg = ansi_default_bg;
      }
      
      if (!encoding_ok)
        return false;
      
      auto ret_fb = TextIO::write_file(fb_filepath, fb_lines, verbose ? 2 : 1);
      auto ret_mat = TextIO::write_file(mat_filepath, mat_lines, verbose ? 2 : 1);
      auto ret_main = TextIO::write_file(file_path, lines);
      return ret_fb && ret_mat && ret_main;
    }
  
  };

}
