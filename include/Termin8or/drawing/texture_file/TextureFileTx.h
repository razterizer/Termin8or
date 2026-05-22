//
//  TextureFileTx.h
//  Termin8or
//
//  Created by Rasmus Anthin on 2026-05-14.
//

#pragma once
#include "../Texture.h"
#include <Core/TextIO.h>
#include <Core/StringHelper.h>


namespace t8
{

  enum class TxGlyphEncoding
  {
    AsciiOnly,                                   // 1 byte per cell.
    EnforceUnicodePreferredAndFallback,          // Store preferred + fallback always.
    TryUnicodePreferredAndFallbackElseAsciiOnly, // Store preferred + fallback if any preferred is non-printable ASCII.
  };
  
  class TextureFileTx
  {
    static bool has_8bit_colors(const Texture& tex)
    {
      for (auto col : tex.fg_colors)
        if (!col.is_color16())
          return true;
      for (auto col : tex.bg_colors)
        if (!col.is_color16())
          return true;
      return false;
    }
    
    static bool has_empty_materials(const Texture& tex)
    {
      for (auto raw_mat : tex.materials_raw)
        if (raw_mat == texture::raw_mat_none)
          return true;
      return false;
    }
    
    static bool has_non_ascii_glyphs(const Texture& tex)
    {
      for (const auto& g : tex.glyphs)
        if (!term::is_printable_ascii(g.preferred))
          return true;
      return false;
    }
    
    static int compute_minimal_version(const Texture& tex, int max_ver = math::get_max<int>())
    {
      int minimal_ver = 10;
      if (minimal_ver < 20 && 20 <= max_ver && has_8bit_colors(tex))
        minimal_ver = 20;
      // VER 2.1: material "none" is serialized as '-' (stored internally as 255).
      if (minimal_ver < 21 && 21 <= max_ver && has_empty_materials(tex))
        minimal_ver = 21;
      if (minimal_ver < 30 && 30 <= max_ver && has_non_ascii_glyphs(tex))
        minimal_ver = 30;
      return minimal_ver;
    }
    
  public:
    
    // File format:
    // size, chars, fg-colors, bg-colors, materials.
    //-------------
    // 3 11
    //
    // /|||::::~~~
    // |:|^..::o~~
    // II#&7/(8---
    //
    // 5A7A4T4BBFF
    // 56633AABBFF
    // TTT5T7T5BBF
    //
    // 122238223CC
    // 1222322255C
    // 2333111CC5T
    //
    // 00001111222
    // 01030110222
    // 44544344222
    
    static bool load_tx(Texture& tex, const std::string& file_path, bool verbose = true)
    {
      std::vector<std::string> lines;
      
      bool ret = TextIO::read_file(file_path, lines);
      if (!ret)
        return false;
      
      int section = 0;
      int r = 0;
      for (const auto& l : lines)
      {
        if (section == 0)
        {
          if (!l.empty())
          {
            if (l.starts_with("VER"))
            {
              auto tokens = str::tokenize(l, { ' ', '.' });
              if (tokens.size() >= 2)
              {
                // VER 1 : Original version. Works without this line.
                // VER 2 : Added 8-bit color support.
                // VER 2.1 : Support for -1 materials (represented as '-') and
                //   fixed missing VER string when saving.
                // Absence of VER line means it is version 1.
                int maj_ver_parsed = std::stoi(tokens[1]);
                int min_ver_parsed = tokens.size() == 3 ? std::stoi(tokens[2]) : 0;
                int ver_parsed = maj_ver_parsed*10 + min_ver_parsed;
                if (ver_parsed <= Texture::compatible_version_until_and_including)
                  tex.ver = ver_parsed;
                else
                {
                  std::cerr << "ERROR in TextureFileAnsi::load_tx() : Incompatible texture version: version = " + tokens[1] << '\n';
                  return false;
                }
              }
              else
                tex.ver = 10; // Absence of VER line means it is version 1.0 -> 10.
            }
            else
            {
              std::istringstream iss(l);
              iss >> tex.size.r >> tex.size.c;
              tex.area = tex.size.r * tex.size.c;
              tex.glyphs.resize(tex.area, ' ');
              tex.fg_colors.resize(tex.area, Color16::Default);
              tex.bg_colors.resize(tex.area, Color16::Transparent2);
              tex.materials_raw.resize(tex.area, texture::raw_mat_none);
            }
            r++;
          }
          else if (r == 1 || r == 2) // 2 if there is a VER line.
          {
            section = 1;
            r = 0;
          }
          else if (r > 2)
          {
            std::cerr << "ERROR in TextureFileAnsi::load_tx() : Incorrect number of header lines.\n";
          }
        }
        else if (section == 1)
        {
          if (!l.empty())
          {
            int l_idx = 0;
            for (int c = 0; c < tex.size.c; ++c)
              tex.glyphs[tex.index(r, c)].parse(l, l_idx, tex.ver <= 21, verbose);
            r++;
          }
          else if (r > 0)
          {
            section = 2;
            r = 0;
          }
        }
        else if (section == 2)
        {
          if (!l.empty())
          {
            int l_idx = 0;
            for (int c = 0; c < tex.size.c; ++c)
              tex.fg_colors[tex.index(r, c)].parse(l, l_idx, true, verbose);
            r++;
          }
          else if (r > 0)
          {
            section = 3;
            r = 0;
          }
        }
        else if (section == 3)
        {
          if (!l.empty())
          {
            int l_idx = 0;
            for (int c = 0; c < tex.size.c; ++c)
              tex.bg_colors[tex.index(r, c)].parse(l, l_idx, true, verbose);
            r++;
          }
          else if (r > 0)
          {
            section = 4;
            r = 0;
          }
        }
        else if (section == 4)
        {
          if (!l.empty())
          {
            int l_idx = 0;
            for (int c = 0; c < tex.size.c; ++c)
              tex.materials_raw[tex.index(r, c)] = texture::str_to_raw_mat(l, l_idx);
            r++;
          }
          else if (r > 0)
          {
            section = 5;
            r = 0;
          }
        }
      }
      return true;
    }
    
    static bool save_tx(const Texture& tex, const std::string& file_path, TxGlyphEncoding encoding_mode = TxGlyphEncoding::AsciiOnly)
    {
      std::vector<std::string> lines;
      
      // Set the lowest supported version.
      int ver = compute_minimal_version(tex, 21);
      
      bool unicode = has_non_ascii_glyphs(tex);
      switch (encoding_mode)
      {
        case TxGlyphEncoding::AsciiOnly:
          break;
        case TxGlyphEncoding::TryUnicodePreferredAndFallbackElseAsciiOnly:
          if (!unicode)
            encoding_mode = TxGlyphEncoding::AsciiOnly;
          else
          ver = 30;
          break;
        case TxGlyphEncoding::EnforceUnicodePreferredAndFallback:
          ver = 30;
          break;
      }
      
      int maj_ver = ver / 10;
      int min_ver = ver % 10;
      lines.emplace_back("VER " + std::to_string(maj_ver) + "." + std::to_string(min_ver));
      
      std::ostringstream oss;
      oss << tex.size.r << " " << tex.size.c;
      lines.emplace_back(oss.str());
      lines.emplace_back("");
      for (int r = 0; r < tex.size.r; ++r)
      {
        std::string curr_line;
        for (int c = 0; c < tex.size.c; ++c)
          curr_line += tex.glyphs[tex.index(r, c)].str(encoding_mode == TxGlyphEncoding::AsciiOnly);
        lines.emplace_back(curr_line);
      }
      lines.emplace_back("");
      for (int r = 0; r < tex.size.r; ++r)
      {
        std::string curr_line;
        for (int c = 0; c < tex.size.c; ++c)
          curr_line += tex.fg_colors[tex.index(r, c)].str(true);
        lines.emplace_back(curr_line);
      }
      lines.emplace_back("");
      for (int r = 0; r < tex.size.r; ++r)
      {
        std::string curr_line;
        for (int c = 0; c < tex.size.c; ++c)
          curr_line += tex.bg_colors[tex.index(r, c)].str(true);
        lines.emplace_back(curr_line);
      }
      lines.emplace_back("");
      for (int r = 0; r < tex.size.r; ++r)
      {
        std::string curr_line;
        for (int c = 0; c < tex.size.c; ++c)
          curr_line += texture::raw_mat_to_str(tex.materials_raw[tex.index(r, c)]);
        lines.emplace_back(curr_line);
      }
      
      return TextIO::write_file(file_path, lines);
    }
  };
  
}
