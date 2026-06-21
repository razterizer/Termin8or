//
//  TextureFile_tests.h
//  Termin8or
//
//  Created by Rasmus Anthin on 2026-05-27.
//

#pragma once
#include "drawing/TextureFile.h"
#include <cassert>
#include <cstdio>
#include <filesystem>
#include <string>

namespace texture_file
{

  void test_material_encoding()
  {
    using namespace t8;
    
    assert(texture::encode_raw_material(texture::mat_none) == texture::raw_mat_none);
    assert(texture::encode_raw_material(-7) == texture::raw_mat_none);
    assert(texture::encode_raw_material(0) == 0);
    assert(texture::encode_raw_material(254) == 254);
    assert(texture::encode_raw_material(300) == 254);
    
    assert(texture::decode_raw_material(texture::raw_mat_none) == texture::mat_none);
    assert(texture::decode_raw_material(0) == 0);
    assert(texture::decode_raw_material(254) == 254);
    
    const std::vector<std::pair<uint8_t, std::string>> cases {
      {   0, "0"  },
      {   9, "9"  },
      {  10, "A"  },
      {  35, "Z"  },
      {  36, "a"  },
      {  61, "z"  },
      {  62, ".0" },
      {  97, ".Z" },
      {  98, ".a" },
      { 123, ".z" },
      { 124, ",0" },
      { 185, ",z" },
      { 186, ":0" },
      { 247, ":z" },
      { 248, ";0" },
      { 254, ";6" },
      { 255, "-"  },
    };
    
    for (const auto& [raw_mat, str] : cases)
    {
      assert(texture::raw_mat_to_str(raw_mat) == str);
      int pos = 0;
      assert(texture::str_to_raw_mat(str, pos) == raw_mat);
      assert(pos == static_cast<int>(str.size()));
    }
    
    int pos = 0;
    assert(texture::str_to_raw_mat(";7", pos) == texture::raw_mat_none);
    assert(pos == 2);
  }

  void test_tx_roundtrip_preserves_unicode_and_materials()
  {
    using namespace t8;
    
    Texture tex { 2, 3 };
    tex.set_textel_glyph(0, 0, Glyph { U'█', '#' });
    tex.set_textel_fg_color(0, 0, RGB6(1, 2, 3));
    tex.set_textel_bg_color(0, 0, Gray24(23));
    tex.set_textel_material(0, 0, 254);
    
    tex.set_textel_glyph(0, 1, Glyph { U'♥', 'v' });
    tex.set_textel_fg_color(0, 1, Color16::Red);
    tex.set_textel_bg_color(0, 1, Color16::Transparent2);
    tex.set_textel_material(0, 1, texture::mat_none);
    
    tex.set_textel_glyph(1, 2, Glyph { 'Z' });
    tex.set_textel_fg_color(1, 2, Color16::Green);
    tex.set_textel_bg_color(1, 2, Color16::Black);
    tex.set_textel_material(1, 2, 62);
    
    const auto path = (std::filesystem::temp_directory_path() /
                       "termin8or_tx_roundtrip_test.tx").string();
    
    assert(TextureFile::save(tex, path));
    
    Texture loaded;
    assert(TextureFile::load(loaded, path, false));
    std::remove(path.c_str());
    
    assert(loaded.size == tex.size);
    assert(loaded.area == tex.area);
    assert(loaded.ver == 30);
    
    for (int i = 0; i < tex.area; ++i)
    {
      assert(loaded.glyphs[i] == tex.glyphs[i]);
      assert(loaded.fg_colors[i] == tex.fg_colors[i]);
      assert(loaded.bg_colors[i] == tex.bg_colors[i]);
      assert(loaded.materials_raw[i] == tex.materials_raw[i]);
    }
  }

  void test_ansi_extension_auto_detection()
  {
    using namespace t8;

    Texture tex { 1, 1 };
    tex.set_textel_glyph(0, 0, Glyph { U'█', '#' });

    for (const auto& ext : { "asc", "nfo", "ASC", "NFO" })
    {
      const auto path = (std::filesystem::temp_directory_path() /
                         ("termin8or_ansi_extension_test." + std::string(ext))).string();

      assert(TextureFile::save(tex, path, TextureFileFormat::Auto, false));

      Texture loaded;
      assert(TextureFile::load(loaded, path, false));

      std::remove(path.c_str());
      std::remove((path + ".fb").c_str());
      std::remove((path + ".mat").c_str());

      assert(loaded.size == tex.size);
      assert(loaded.glyphs[0] == tex.glyphs[0]);
    }
  }

  void unit_tests()
  {
    test_material_encoding();
    test_tx_roundtrip_preserves_unicode_and_materials();
    test_ansi_extension_auto_detection();
  }
}
