//
//  TextureFile.h
//  Termin8or
//
//  Created by Rasmus Anthin on 2026-05-14.
//

#pragma once
#include "texture_file/TextureFileTx.h"
#include "texture_file/TextureFileAnsi.h"


namespace t8
{

  enum class TextureFileFormat
  {
    Auto,
    Tx,
    Ansi,
  };

  class TextureFile
  {
  
    static TextureFileFormat deduce_file_format(const std::string& file_path)
    {
      auto ext = get_file_ext(file_path);
      if (is_ext_ansi_cp437(ext) || ext == "utf8ans")
        return TextureFileFormat::Ansi;
      if (ext == "tx")
        return TextureFileFormat::Tx;
      return TextureFileFormat::Auto;
    }
    
  public:
    
    static bool load(Texture& tex, const std::string& file_path, bool verbose)
    {
      return load(tex, file_path, TextureFileFormat::Auto, verbose);
    }
    
    static bool load(Texture& tex, const std::string& file_path,
                     TextureFileFormat format = TextureFileFormat::Auto,
                     bool verbose = true,
                     AnsiLoadGlyphEncoding ansi_glyph_encoding = AnsiLoadGlyphEncoding::Auto,
                     Color ansi_default_fg = Color16::Default,
                     Color ansi_default_bg = Color16::Transparent2)
    {
      auto resolved_format = format == TextureFileFormat::Auto ?
      deduce_file_format(file_path) : format;
      
      Texture parsed;
      
      bool ok = false;
      switch (resolved_format)
      {
        case TextureFileFormat::Auto:
          return false;
        case TextureFileFormat::Tx:
          ok = TextureFileTx::load_tx(parsed, file_path, verbose);
          break;
        case TextureFileFormat::Ansi:
          ok = TextureFileAnsi::load_ansi(parsed, file_path, verbose,
                                          ansi_glyph_encoding,
                                          ansi_default_fg, ansi_default_bg);
          break;
      }
      
      if (!ok)
        return false;
      
      tex = std::move(parsed);
      return true;
    }
    
    static bool save(const Texture& tex, const std::string& file_path, bool verbose, TxGlyphEncoding encoding_mode)
    {
      return save(tex, file_path, TextureFileFormat::Auto, verbose, encoding_mode);
    }
    
    static bool save(const Texture& tex, const std::string& file_path,
                     TextureFileFormat format = TextureFileFormat::Auto,
                     bool verbose = true,
                     TxGlyphEncoding encoding_mode = TxGlyphEncoding::AsciiOnly,
                     AnsiSaveGlyphEncoding ansi_glyph_encoding = AnsiSaveGlyphEncoding::AutoPreserveGlyphs,
                     Color ansi_default_fg = Color16::Default,
                     Color ansi_default_bg = Color16::Transparent2)
    {
      auto resolved_format = format == TextureFileFormat::Auto ?
      deduce_file_format(file_path) : format;
      
      switch (resolved_format)
      {
        case TextureFileFormat::Auto:
          return false;
        case TextureFileFormat::Tx:
          return TextureFileTx::save_tx(tex, file_path, encoding_mode);
        case TextureFileFormat::Ansi:
          return TextureFileAnsi::save_ansi(tex, file_path, verbose,
                                            ansi_glyph_encoding,
                                            ansi_default_fg,
                                            ansi_default_bg);
      }
      
      return false;
    }
    
  };
  
}
